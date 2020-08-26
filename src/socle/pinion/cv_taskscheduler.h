// sc_taskscheduler.h _____________________________________________________________________________________________________________
#pragma once
  
#include    "socle/coffer/sc_taskdiary.h"  

//_____________________________________________________________________________________________________________________________
// Struct to setup  HeistSessions on CPUs and  provide them with common services

struct Sc_TaskLedger  : public Sc_TaskScheme
{
    Sc_Unit< uint32_t>          m_CntActive;            // Count of Active Processing Session, used for startup and shutdown
    Sc_Unit< uint32_t>          m_SzRuning;             // Count of sessions running tasks.
    Sc_Unit< uint32_t>          m_SeqSC;
    uint32_t                    m_SzSession;
    bool                        m_StopFlg;
    uint32_t                    m_UpdateMSec;
    TaskStore                   *m_TaskStore;           // common free-pool for tasks.

    std::ostream                *m_OStrmPtr;            // common log stream
    Spinlock                    m_OStrmlock;  

    Sc_TaskLedger( void)
        : m_CntActive( 0), m_SzRuning( 0), m_SzSession( 0), m_StopFlg( false), m_UpdateMSec( 1000), m_TaskStore( NULL), m_OStrmPtr( NULL)
    {} 
    
    ~Sc_TaskLedger( void)
    {
        if ( m_TaskStore)
            delete m_TaskStore;
    }
    
    bool    DoInit( void)
    {
        m_TaskStore = new TaskStore( 91);
        return true;
    }
    
    std::ostream    *OStrm( void) { return m_OStrmPtr; }
    void            SetOStrm( std::ostream *pOStrm) { m_OStrmPtr = pOStrm; }
};
 
//_____________________________________________________________________________________________________________________________
 
struct Cv_HiestQueue  : public Sc_TaskScheme
{    
    //__________________________________________________________________________________________________________________________

    TaskStk                 m_TaskIdStk;
    TaskStk                 m_TempStk;
    Diary                   *m_Diary;           // diary for to all scheme in the 
    TaskCache               m_TaskCache; 
    Spinlock                m_Spinlock;

    bool            DoInit( Diary *diary)
    {
        m_Diary = diary; 
        m_TaskCache.SetStore( diary->m_TaskStore);
        m_TaskIdStk.DoInit( MaxTask);
        m_TempStk.DoInit( MxFnTask);
        return true;
    }  

    TaskId          PopTask( void)  
    {  
        Spinlock::Guard         guard( &m_Spinlock); 
        if ( !m_TaskIdStk.SzStk())
            return Diary::NullTask(); 
        return m_TaskIdStk.PopBack();    
    }

    void            FlushTempTasks( void)  
    {   
        Spinlock::Guard         guard( &m_Spinlock);  
        m_TempStk.TransferTo( &m_TaskIdStk);
    }
    
    void            EnqueueTask( TaskId taskId)  
    {    
        m_TempStk.PushBack( taskId);
    }
    
 
template < typename Rogue,  typename... Args>
    auto            Construct( TaskId succId,  Rogue  rogue, const Args &... args) 
    {  
        if ( succId) 
            m_TaskCache.GetAtFromId( succId)->RaiseSzPred(); 
        uint32_t        sz = m_TaskCache.ProbeSzFree();         
        CV_ERROR_ASSERT( sz && ( m_TaskIdStk.Size() != m_TaskIdStk.SzStk()))         // Too many tasks..
        TaskId      taskId = m_TaskCache.AllocId();
        Task        *task = MakeTask( m_TaskCache.GetAtFromId( taskId), rogue, args...);  
        
        task->SetSuccId( succId);
        return taskId; 
    } 
    
    void            Run( TaskId taskId, Cv_HeistSession *scheme) 
    { 
        Task        *task = m_TaskCache.GetAtFromId( taskId);
        CV_SANITY_ASSERT( !task->SzPred())
        TaskId      succId = task->SuccId();
        task->DoWork( succId, scheme);

        if ( succId) 
        {
            auto    succPred = m_TaskCache.GetAtFromId( succId)->LowerSzPred(); 
            if ( !succPred)
                EnqueueTask( succId);
        }
        
        FlushTempTasks(); 
        CV_SANITY_ASSERT( memset( ( void *) task, 0xCC, SzTask)) 
        m_TaskCache.Discard( taskId);
        return;
    }
};
 
//_____________________________________________________________________________________________________________________________

struct Cv_HeistSession  : public Cv_HiestQueue 
{
    Sc_Unit< uint64_t>       m_Worktime;         // record of useful time spent
    Sc_Unit< uint32_t>       m_Index;
    Sc_Unit< bool>           m_DoneFlg;          // When scheme is over.
    Sc_Unit< uint32_t>       m_SzTaskRuns;
    std::thread             m_Thread;

    Cv_HeistSession( void)
        : m_Index( CV_UINT32_MAX), m_DoneFlg( false), m_SzTaskRuns( 0) 
    {} 
    
    bool    DoInit( Diary *diary, uint32_t index)
    {
        m_Index = index; 
        Cv_HiestQueue::DoInit( diary);
        return true;
    }  

    std::ostream    *OStrm( void) { return m_Diary->OStrm(); }
    uint32_t        SzSession( void) const { return m_Diary->m_SzSession; }
 
    uint32_t        Index( void) const { return m_Index.Get(); } 
    
    bool            IsTaskPending( void) const;

     
    
    TaskId        GrabTask( void) const;
    
    void            Run( TaskId taskId) 
    { 
        auto    val = m_Diary->m_SeqSC.Get() +m_Index.Get();
        Cv_HiestQueue::Run( taskId, this); 
        m_Diary->m_SeqSC.SetSC( val);
    }

    void            RunAll( void)
    { 
        while ( m_TaskIdStk.SzStk() || m_Diary->m_SzRuning.Get())
        {
            m_Diary->m_SzRuning.Incr();
            auto            taskId = PopTask();
            if ( taskId == Diary::NullTask()) 
                taskId = GrabTask();
            
            if ( taskId != Diary::NullTask()) 
            {
                m_SzTaskRuns.Incr();
                Run( taskId);
            }
            m_Diary->m_SzRuning.Decr();            
        }
        return;
    }
     
    void            DoStart( void)
    {
        m_Diary->m_CntActive.Incr();
        while ( m_Diary->m_CntActive.Get() != m_Diary->m_SzSession)
            std::this_thread::yield();
    }

    void            DoStop( void)
    { 
        if ( m_Diary->OStrm())
        {
            Spinlock::Guard         guard( &m_Diary->m_OStrmlock); 
            *m_Diary->OStrm() << "Stoping " << m_Index.Get() << " Task: " << m_SzTaskRuns.Get()  <<  '\n';
            m_Diary->OStrm()->flush();
        }
        m_Diary->m_CntActive.Decr();
        m_DoneFlg = true;
    }
  

    void            DoExecute( void)
    {  
        FlushTempTasks();
        DoStart();
        while ( IsTaskPending())
        {
            RunAll();
            //std::this_thread::sleep_for( std::chrono::milliseconds(1));
        }
        DoStop();
        return;
    } 

    bool            DoLaunch( void)
    {
        m_Thread  = std::thread( &Cv_HeistSession::DoExecute, this); 
        return true;
    }

    bool            DoJoin( void)
    {
        m_Thread.join();
        return true;
    }

    auto            Ops( uint16_t succ) 
    {
        return Cv_HeistOps< Cv_HeistSession>( succ, this);
    }
}; 

//_____________________________________________________________________________________________________________________________

struct Sc_TaskScheduler :  public Sc_TaskLedger
{    
    uint32_t                                    m_MxSession;
    uint32_t                                    m_LastGrab;
    Cv_HeistSession                             m_Session; 
    Cv_ArrStk< Cv_HeistSession *, MxThread>     m_HeistSessions;
    
    struct TaskLoadCmp
    {
         bool    operator()( Cv_HeistSession *s1, Cv_HeistSession *s2) const
            {   return s1->m_TaskIdStk.SzStk() > s2->m_TaskIdStk.SzStk(); }
    };

public:
    Sc_TaskScheduler( uint32_t mxSession) 
        : m_MxSession( mxSession), m_LastGrab( 0)
    {} 

    ~Sc_TaskScheduler( void)
    {
        for ( uint32_t i = 0; i < m_MxSession; ++i)
            delete m_HeistSessions[ i];
    }

    Cv_HeistSession  *CurSession( void) { return &m_Session; }    

    bool            DoInit( void)
    {
        bool    flg = Sc_TaskLedger::DoInit();
        m_HeistSessions.IncrFill( m_MxSession +1);
        for ( uint32_t i = 0; flg && ( i < m_HeistSessions.SzStk()); ++i)
        {
            m_HeistSessions[ i] = ( i < m_MxSession) ? new Cv_HeistSession() : &m_Session;
            flg = m_HeistSessions[ i]->DoInit( this, i);
        }
        if ( !flg)
            return false;
        m_SzSession = m_HeistSessions.SzStk(); 
        return flg;
    }
    
    bool    IsTaskPending( void) const 
    {
        uint32_t        sz = m_HeistSessions.SzStk();
        for ( uint32_t i = 0; i < sz; ++i)
            if ( m_HeistSessions[ i]->m_TaskIdStk.SzStk())
                return true;
        return false;
    }

    TaskId          GrabTask( void)   
    { 
        uint32_t        sz = uint32_t( m_HeistSessions.SzStk());
        for ( uint32_t i = 0; i < sz; ++i, ++m_LastGrab)
        {
            Cv_HeistSession  *scheme = m_HeistSessions[ m_LastGrab % sz]; 
            if ( scheme->m_TaskIdStk.SzStk())
                return scheme->PopTask();
        }
        return NullTask();
    }

    bool    DoLaunch( void)
    {  
        bool    flg = true; 
        for ( uint32_t i = 0; flg && ( i < ( m_HeistSessions.SzStk() -1)); ++i)
            flg = m_HeistSessions[ i]->DoLaunch();
        if ( !flg)
            return false;  
        m_Session.DoExecute(); 
        for ( uint32_t i = 0; flg && ( i < ( m_HeistSessions.SzStk() -1)); ++i)
            m_HeistSessions[ i]->DoJoin();
        return true;
    }

    static uint32_t     SzLogicalCPU( void); 
}; 

//_____________________________________________________________________________________________________________________________

inline bool    Cv_HeistSession::IsTaskPending( void) const 
{
    return static_cast < Sc_TaskScheduler*>( m_Diary)->IsTaskPending(); 
}

//_____________________________________________________________________________________________________________________________

inline Cv_HeistSession::TaskId          Cv_HeistSession::GrabTask( void) const
{ 
    return static_cast < Sc_TaskScheduler*>( m_Diary)->GrabTask(); 
}

//_____________________________________________________________________________________________________________________________
