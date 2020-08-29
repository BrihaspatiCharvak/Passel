// sc_atmunit.h -------------------------------------------------------------------------------------------------------------------
#pragma once

#include    "socle/tenor/sc_include.h" 

//---------------------------------------------------------------------------------------------------------------------------------
// memory_order_relaxed : Each location has a total modification order and  same thread on the same memory location are not reordered with respect to the modification order 

template < typename StorT, typename AtmT =std::true_type>
class Sc_Unit
{ 
public:
    typedef StorT       Stor;
    typedef AtmT        Atm;
private:   
    std::atomic< Stor>          m_Value;
  
public:
    Sc_Unit( const Stor &value = Stor()) 
        :  m_Value( value)
    {} 
  
    Sc_Unit( const Sc_Unit &atm) 
        :  m_Value( Stor( atm.m_Value))
    {} 
     
    Stor    Get( void) const 
    {  
        return Stor( m_Value.load( std::memory_order_relaxed)); // On x86_64 every load has aqcuire semantic, so load with acquire semantic is same as one with relaxed semantic 
    }  
    
    Stor    Set( Stor t)
    {  
        m_Value.store( t, std::memory_order_relaxed);           // x86_64 the costs are same as for simple read store.
        return t;
    }    
     
    Stor    SetSC( Stor t)
    {  
        m_Value.store( t, std::memory_order_seq_cst);           // as of now,we have only one memory_order_seq_cst call, after every task run.
        return t;
    } 

    Sc_Unit     &operator=( const Sc_Unit &t)
    {
        Set( t.Get());
        return *this;        
    }
    
    auto            Incr( void)  { return ++m_Value;  } 
    auto            Incr( Stor t)  { return m_Value += t;  } 

    auto            Decr( void)  { return --m_Value;  }
    auto            Decr( Stor t)  { return m_Value -= t;  } 

 
    Sc_Unit     &operator+=( const Stor &t) 
    {
        m_Value += t;
        return *this;
    }
    
    bool            operator<( const Sc_Unit &atm) const  { return Get() < atm.Get(); }
    bool            operator==( const Sc_Unit &atm) const  { return Get() == atm.Get(); }

    Stor    Diff( const Sc_Unit &t) 
    {
        return m_Value -t.m_Value;
    }

    Stor    Exchange( const Stor &oldValue, const Stor &newValue)
    {
        Stor    storedValue = oldValue;                         // oldValue is what caller had assumed to be StoredValue to compute the newValue
        do {                                                    // Keep trying until storedValue in Atomic is no longer oldValue
            if (  storedValue != oldValue)
                return storedValue;                             // someone else changed the content to expectedValue; We might be OK or need to recompute the newValue
        } while( !m_Value.compare_exchange_weak( expectedValue, newValue));
        return newValue;                                        // desired value is written to memory
    }
};
 
//---------------------------------------------------------------------------------------------------------------------------------

template < typename StorT>
class Sc_Unit< StorT, std::false_type>
{

public:
    typedef StorT       Stor; 

private:
    Stor      m_Value;
 
public:
    Sc_Unit( const Stor &value = Stor()) 
        :  m_Value( value)
    {} 
 
    Stor    Get( void) const  {  return Stor( m_Value); }   
    Stor    Set( Stor t) { return m_Value = t; }    

    Sc_Unit     &operator=( const Sc_Unit &t)
    {
        Set( t.Get());
        return *this;        
    }
    
    void            Incr( void)  { ++m_Value;  } 
    void            Incr( Stor t)  { m_Value += t;  } 

    void            Decr( void)  { --m_Value;  }
    void            Decr( Stor t)  { m_Value -= t;  } 

 template < typename T>
    Sc_Unit     &operator+=( const T &t) 
    {
        m_Value += t;
        return *this;
    }
    
    Stor    Diff( const Sc_Unit &t) 
    {
        return m_Value -t.m_Value;
    }
};

//---------------------------------------------------------------------------------------------------------------------------------
 
template < typename Stor>
struct Cv_AUnit : public Sc_Unit< Stor, std::true_type>
{
    Cv_AUnit( const Stor &value = Stor()) 
        :  Sc_Unit< Stor, std::true_type>( value)
    {} 
};
 
template < typename Stor>
struct Cv_NUnit : public Sc_Unit< Stor, std::false_type>
{
    Cv_NUnit( const Stor &value = Stor()) 
        :  Sc_Unit< Stor, std::false_type>( value)
    {} 
};

//---------------------------------------------------------------------------------------------------------------------------------

template < typename Atm>
class Sc_Spinlock
{     
    std::atomic_flag m_Flag; 

public:
    Sc_Spinlock( void)  
        : m_Flag() 
    {} 

    void Lock()
    { 
        while ( m_Flag.test_and_set())
        {}      
    }

    void Unlock()
    { 
        m_Flag.clear(); 
    }

    struct Guard
    {
        Sc_Spinlock     *m_Lock;

        Guard( Sc_Spinlock *lck)
            : m_Lock( lck)
        {
            m_Lock->Lock();
        }

        ~Guard()
        {
            m_Lock->Unlock();
        }                
    };  
};

//---------------------------------------------------------------------------------------------------------------------------------

template <>
class Sc_Spinlock<std::false_type>
{       

public:
    Sc_Spinlock<std::false_type>( void)  
    {} 

    void Lock()
    {  
    }

    void Unlock()
    { 
    }

    struct Guard
    { 
        Guard( Sc_Spinlock<std::false_type> *) 
        { 
        }

        ~Guard()
        { 
        }                
    };  
};


//---------------------------------------------------------------------------------------------------------------------------------