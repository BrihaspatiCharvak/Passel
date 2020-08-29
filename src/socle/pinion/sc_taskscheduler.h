// sc_taskscheduler.h -------------------------------------------------------------------------------------------------------------
#pragma once
  
#include    "socle/pinion/sc_taskdiary.h"  

//---------------------------------------------------------------------------------------------------------------------------------
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
 
//---------------------------------------------------------------------------------------------------------------------------------

struct Cv_TaskQueue  : public Sc_TaskScheme
{      
    TaskStk                 m_TaskIdStk;
    TaskStk                 m_TempStk;
    Ledger                  *m_Diary;           // diary for to all scheme in the 
    TaskCache               m_TaskCache; 
    Spinlock                m_Spinlock;

    bool            DoInit( Ledger *diary)
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
            return Ledger::NullTask(); 
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
        SC_ERROR_ASSERT( sz && ( m_TaskIdStk.Size() != m_TaskIdStk.SzStk()))         // Too many tasks..
        TaskId      taskId = m_TaskCache.AllocId();
        Task        *task = MakeTask( m_TaskCache.GetAtFromId( taskId), rogue, args...);  
        
        task->SetSuccId( succId);
        return taskId; 
    } 
    
    void            Run( TaskId taskId, Sc_TaskSession *scheme) 
    { 
        Task        *task = m_TaskCache.GetAtFromId( taskId); 
        TaskId      succId = task->SuccId();
        task->DoWork( succId, scheme);

        if ( succId) 
        {
            auto    succPred = m_TaskCache.GetAtFromId( succId)->LowerSzPred(); 
            if ( !succPred)
                EnqueueTask( succId);
        }
        
        FlushTempTasks();  
        m_TaskCache.Discard( taskId);
        return;
    }
};
 
//---------------------------------------------------------------------------------------------------------------------------------


struct Sc_TaskSession  : public Cv_TaskQueue 
{
    Sc_Unit< uint64_t>       m_Worktime;         // record of useful time spent
    Sc_Unit< uint32_t>       m_Index;
    Sc_Unit< bool>           m_DoneFlg;          // When scheme is over.
    Sc_Unit< uint32_t>       m_SzTaskRuns;
    std::thread             m_Thread;

    Sc_TaskSession( void)
        : m_Index( SC_UINT32_MAX), m_DoneFlg( false), m_SzTaskRuns( 0) 
    {} 
    
    bool    DoInit( Ledger *diary, uint32_t index)
    {
        m_Index = index; 
        Cv_TaskQueue::DoInit( diary);
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
        Cv_TaskQueue::Run( taskId, this); 
        m_Diary->m_SeqSC.SetSC( val);
    }

    void            RunAll( void)
    { 
        while ( m_TaskIdStk.SzStk() || m_Diary->m_SzRuning.Get())
        {
            m_Diary->m_SzRuning.Incr();
            auto            taskId = PopTask();
            if ( taskId == Ledger::NullTask()) 
                taskId = GrabTask();
            
            if ( taskId != Ledger::NullTask()) 
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
        m_Thread  = std::thread( &Sc_TaskSession::DoExecute, this); 
        return true;
    }

    bool            DoJoin( void)
    {
        m_Thread.join();
        return true;
    }
 
}; 

//---------------------------------------------------------------------------------------------------------------------------------


struct Sc_TaskScheduler :  public Sc_TaskLedger
{     
    uint32_t                                    m_LastGrab;
    Sc_TaskSession                              m_Session; 
    Sc_AtmVec< Sc_TaskSession *>                m_HeistSessions;
    
    struct TaskLoadCmp
    {
         bool    operator()( Sc_TaskSession *s1, Sc_TaskSession *s2) const
            {   return s1->m_TaskIdStk.SzStk() > s2->m_TaskIdStk.SzStk(); }
    };

public:
    Sc_TaskScheduler( uint32_t mxSession) 
        : m_LastGrab( 0)
    {
        m_HeistSessions.DoInit( mxSession +1); 
    } 

    ~Sc_TaskScheduler( void)
    {
        for ( uint32_t i = 1; i < m_HeistSessions.SzStk(); ++i)
            delete m_HeistSessions.At( i);
    }

    Sc_TaskSession  *CurSession( void) { return &m_Session; }    

    bool            DoInit( void)
    {
        bool    flg = Sc_TaskLedger::DoInit(); 
        for ( uint32_t i = 0; flg && ( i < m_HeistSessions.Size()); ++i)
        {
            auto    *session = i ? new Sc_TaskSession() : &m_Session;
            m_HeistSessions.PushBack( session);
            flg = session->DoInit( this, i);
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
            if ( m_HeistSessions.At( i)->m_TaskIdStk.SzStk())
                return true;
        return false;
    }

    TaskId          GrabTask( void)   
    { 
        uint32_t        sz =  m_HeistSessions.SzStk();
        for ( uint32_t i = 0; i < sz; ++i, ++m_LastGrab)
        {
            Sc_TaskSession  *scheme = m_HeistSessions.At( m_LastGrab % sz); 
            if ( scheme->m_TaskIdStk.SzStk())
                return scheme->PopTask();
        }
        return NullTask();
    }

    bool    DoLaunch( void)
    {  
        bool    flg = true; 
        for ( uint32_t i = 1; flg && ( i <  m_HeistSessions.SzStk()); ++i)
            flg = m_HeistSessions.At( i)->DoLaunch();
        if ( !flg)
            return false;  
        m_Session.DoExecute(); 
        for ( uint32_t i = 1; flg && ( i < m_HeistSessions.SzStk()); ++i)
            m_HeistSessions.At( i)->DoJoin();
        return true;
    }

    static uint32_t     SzLogicalCPU( void); 
}; 

//---------------------------------------------------------------------------------------------------------------------------------


inline bool    Sc_TaskSession::IsTaskPending( void) const 
{
    return static_cast < Sc_TaskScheduler*>( m_Diary)->IsTaskPending(); 
}

//---------------------------------------------------------------------------------------------------------------------------------


inline Sc_TaskSession::TaskId          Sc_TaskSession::GrabTask( void) const
{ 
    return static_cast < Sc_TaskScheduler*>( m_Diary)->GrabTask(); 
}

//---------------------------------------------------------------------------------------------------------------------------------

