// sc_tasktools.h ----------------------------------------------------------------------------------------------------------------

 #include    "socle/pinion/sc_taskdiary.h"  

//---------------------------------------------------------------------------------------------------------------------------------
 
template < typename Queue>
class Sc_TaskTools  : public Sc_TaskScheme
{ 
    Queue           *m_Queue;
    uint32_t        m_MxAggr;                   // Max number of aggregators
    uint16_t        m_Succ;
    
public:
    Sc_TaskTools( uint16_t succ, Queue *queue)
        : m_Queue( queue), m_MxAggr( 2 * m_Queue->SzSession()), m_Succ( succ)
    {
    }
    
    uint32_t    SzAggr( void) const { return m_MxAggr; } 
    uint32_t    DefSzSection( uint32_t sz) const { return sz/m_MxAggr; }

    uint32_t    StartInd( uint32_t aggrInd, uint32_t sz) const { return  aggrInd * DefSzSection( sz);  }
    uint32_t    SzSection( uint32_t aggrInd, uint32_t sz) const { return (( aggrInd+1) != m_MxAggr) ? DefSzSection( sz) : sz -StartInd( aggrInd, sz);  } 

    void        BuildKnots( Cv_KnotVec *pKnots, uint32_t b, uint32_t sz) const 
    { 
        pKnots->InitKnots( m_MxAggr);
        for ( uint32_t i = 0, beg = b; i < m_MxAggr; ++i, beg += DefSzSection( sz) )
            pKnots->AddKnots( beg);
        pKnots->CapOffKnots( b +sz);
        return;
    }

template < typename Lambda, typename... Args>
    void    ForTasks( uint32_t b, uint32_t sz, const Lambda &lambda, const Args &... args)
    {
        for ( uint32_t i = 0, beg = b; i < m_MxAggr; ++i, beg += DefSzSection( sz) )
        {
            uint32_t    szSec = SzSection( i, sz); 
            auto        subTask = m_Queue->Construct( m_Succ, [=]( uint16_t succ, Queue *queue) {  
                Cv_Aid::ForAll( beg, szSec, lambda, i, args...);
            }); 
            m_Queue->EnqueueTask( subTask); 
        }
        return;
    }

template < typename Lambda, typename... Args>
    void    ForTasksUni( uint32_t b, uint32_t sz, const Lambda &lambda, const Args &... args)       // For debugs
    {
        auto        subTask = m_Queue->Construct( m_Succ, [=]( uint16_t succ, Queue *queue) {  
            Cv_Aid::ForAll( b, sz, lambda, 0, args...);
        }); 
        m_Queue->EnqueueTask( subTask); 
        return;
    }
};

//---------------------------------------------------------------------------------------------------------------------------------
 