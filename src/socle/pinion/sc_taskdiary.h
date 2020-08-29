// cv_taskdiary.h -------------------------------------------------------------------------------------------------------------------
#pragma once 

#include    "socle/coffer/sc_freestore.h" 

struct Sc_TaskLedger;
struct Sc_TaskSession;  
struct Sc_TaskScheduler; 

//---------------------------------------------------------------------------------------------------------------------------------
 

struct Sc_TaskScheme
{
    typedef  uint16_t       TaskId;                 // we should be fine with 16K possible threads, if more are needed 
                                                    // better change the user algorithm design. For limited number of CPUs higher granular errand are a drag
    enum {
        MxThread            = 64,                   // Max number of threads
        SzTask            = 64,                                  
        MxFnTask          = 64,                   // Max number of Tasks an Task can enqueue.                        
        MaxTask           = std::numeric_limits< TaskId>::max(), 
    };
    
    // base errand class :
    class   Task
    {  
        Sc_Unit< TaskId>        m_SuccId;           // the Id of errand to run after this
        Sc_Unit< uint16_t>      m_SzPred;           // number of errands still to run before this task can be taken up
                                                    // the scheduler checks the Succ after this errand completes and 
                                                    // queues it it the Succ does not have any Pred left to run. ie, m_SzPred == 0                     

    public:
        typedef Sc_TaskLedger     Ledger;

        Task( void)
            :  m_SuccId( NullTask()), m_SzPred( 0) 
        {}

        virtual void        DoWork( TaskId succId, Sc_TaskSession *scheme) const = 0;    // the core work for the errand

        uint16_t            SuccId( void) const { return m_SuccId.Get(); }
        void                SetSuccId( uint16_t succId) { m_SuccId.Set( succId); }

        uint16_t            SzPred( void) const { return m_SzPred.Get(); }
        
        auto                RaiseSzPred( void) {  return m_SzPred.Incr(); }
        auto                LowerSzPred( void) {  return m_SzPred.Decr(); }
        
        void                Ship( void) { return ; }
    };

    typedef Sc_FreeStore< Task, uint16_t, MaxTask, SzTask>          TaskStore;
    typedef Sc_FreeCache< 256, TaskStore>                           TaskCache;
    typedef Sc_AtmVec< TaskId>                                      TaskStk; 

    typedef Sc_Spinlock< std::true_type>                            Spinlock;
 
    constexpr static TaskId                                         NullTask( void) { return TaskId( 0); }

    typedef Sc_TaskLedger                                           Ledger;
    
    //__________________________________________________________________________________________________________________________

template < typename Rogue >
    struct TaskJob : public Task
    {
        typedef typename Task::Ledger      Ledger;
            
        Rogue      m_Task;

        TaskJob( const Rogue &rogue)
            : m_Task( rogue)
        {
            static_assert( sizeof( TaskJob) <= Ledger::SzTask);
        }

        void        DoWork( TaskId succId, Sc_TaskSession *scheme) const
        {
            m_Task( succId, scheme); 
        }
    };

    //__________________________________________________________________________________________________________________________

template < typename Rogue >
    struct TaskPtrJob : public Task
    {
        typedef typename Task::Ledger      Ledger;
        
        Rogue      *m_Task;

        TaskPtrJob(  Rogue *rogue)
            : m_Task( rogue)
        {
            static_assert( sizeof( TaskPtrJob) <= Ledger::SzTask);
        }

        void        DoWork( TaskId succId, Sc_TaskSession *scheme) const
        {
            (*m_Task)( succId, scheme); 
        }
    };

    //__________________________________________________________________________________________________________________________

template < typename Rogue, typename Call, typename... Args>
    struct TaskCall : public Task
    {
        
        TaskCall( Rogue *rogue, const Call &callFn, Args... args)
        {}
    };

    //__________________________________________________________________________________________________________________________

template < typename Rogue, typename Call, typename Arg >
    struct TaskCall<Rogue, Call, Arg> : public Task
    { 
        typedef typename Task::Ledger      Ledger;
        
        Rogue       *m_Task;
        Call        m_Call;
        Arg         m_Arg;

        TaskCall( Rogue *rogue, const Call &callFn, const Arg &arg)
            : m_Task( rogue), m_Call( callFn), m_Arg( arg)
        { 
            static_assert( sizeof( TaskCall) <= Ledger::SzTask);
        }

        void        DoWork( TaskId succId, Sc_TaskSession *scheme) const
        {
            (m_Task->*m_Call)( succId, scheme, m_Arg); 
        }
    };

    //__________________________________________________________________________________________________________________________
    // Functor for 
    
template < typename Rogue, typename Call >
    struct TaskCall<Rogue, Call> : public Task
    { 
        typedef typename Task::Ledger      Ledger;
        
        Rogue       *m_Task;
        Call        m_Call; 

        TaskCall( Rogue *rogue, const Call &callFn)
            : m_Task( rogue), m_Call( callFn)
        {
            static_assert( sizeof( TaskCall) <= Ledger::SzTask);
        }

        void        DoWork( TaskId succId, Sc_TaskSession *scheme) const
        {
            (m_Task->*m_Call)( succId, scheme); 
        }
    }; 

template < typename  Rogue>  
    static Task   *MakeTask( void *ptr, const Rogue &rogue)
    {
        return new (ptr) TaskJob( rogue);
    }

template < typename  Rogue>  
    static Task   *MakeTask( void *ptr, Rogue *rogue)
    {
        return new (ptr) TaskPtrJob( rogue);
    }

template < typename  Rogue, typename Call, typename... Args>  
    static Task   *MakeTask( void *ptr, Rogue *rogue, const Call &call, const Args &... args)
    {
        return new (ptr) TaskCall<Rogue, Call, Args...>( rogue, call, args...);
    } 

};

//---------------------------------------------------------------------------------------------------------------------------------


