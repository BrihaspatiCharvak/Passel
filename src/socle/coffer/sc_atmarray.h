// sc_atmarray.h _____________________________________________________________________________________________________________
#pragma once
 
#include    "socle/coffer/sc_atmtrail.h" 

//_____________________________________________________________________________________________________________________________

template <  typename StorT, typename SzTypeT = uint32_t, typename AtmT = std::true_type>
class Sc_AtmArr : public Sc_AtmIter< Sc_AtmArr< StorT, SzTypeT, AtmT>, StorT, SzTypeT, AtmT>
{  
public:
    typedef  StorT                  Stor; 
    typedef  SzTypeT                SzType; 
    typedef  AtmT                   Atm;   
    typedef  Sc_Unit< SzType, Atm>  AtmSize; 
    typedef  Sc_Unit< Stor, Atm>    AtmStor;

protected:
    typedef Sc_AtmIter< Sc_AtmArr< Stor, SzType, Atm>, Stor, SzType, Atm>    Base; 

    AtmSize        m_Sz;
    AtmStor        *m_Vec; 
    
    Sc_AtmArr( const Sc_AtmArr &av)
        : m_Sz( av.m_Sz), m_Vec( av.m_Vec)
    {}

public:
    Sc_AtmArr( void)
        : m_Sz( 0), m_Vec( NULL) 
    {} 
    
    ~Sc_AtmArr( void)
    {
        if ( Size()) 
            delete [] m_Vec; 
    }

    AtmStor        *Vec( void) { return m_Vec; }
    const AtmStor  *Vec( void) const { return m_Vec; }
    
    void        DoInit( SzType sz)
    {
        m_Sz.Set( sz);
        m_Vec = new AtmStor[ sz];  
    }

    void        DoInit( SzType sz, Stor iVal)
    {
        DoInit( sz);
        for ( SzType i = 0; i < sz; ++i) 
            this->SetAt( i, iVal); 
    }

    void        DoIndicize( SzType sz)
    {
        DoInit( sz);  
        for ( SzType i = 0; i < sz; ++i) 
            this->SetAt( i, i); 
    }
 
    void        Resize( SzType sz)
    {
        AtmStor   *tVec = m_Vec;
        m_Vec = new AtmStor[ sz]; 
        SzType    tSz = std::min( sz, Size());
        for ( SzType i = 0; i < tSz; ++i) 
            this->SetAt( i, tVec[ i].Get()); 
        delete [] tVec;
        m_Sz.Set( sz);
    }  

    SzType          Size( void) const { return m_Sz.Get(); }  
    void            ShrinkUpTo( SzType sz) { m_Sz.Set( sz); }
  

template < typename Lambda, typename... Args>   
    void    TraverseAll( const Lambda &lambda, const Args &... args) const 
    { 
        Cv_Aid::ForAll( 0,  Size(), lambda, args...); 
    }

template < typename Iter, typename Lambda, typename... Args>   
    void    Splice( Iter *iter, const Lambda &filter, const Args &... args)  
    {
        Sc_AtmArr     old = SELF;
        SzType          oldSz = old.Size();
        SzType          sz = m_Sz.Set( old.Size() + iter->Size());
        m_Vec = new AtmStor[ sz];
        SzType          vInd = 0;
        old.TraverseAll( [&]( SzType i) {  
            SetAt( vInd++, old.At( i));
        });
        iter->TraverseAll( [&]( SzType i) {  
            auto    val = iter->At( i);
            if ( filter( val, args...))
                SetAt( vInd++, val);
        });
        m_Sz.Set( vInd);
        return;
    }

    void    Dump( std::ostream &ostr)
    {
        TraverseAll( [&]( SzType sv) {
            ostr << At( sv) << (( sv == ( Size() -1)) ? '\n' : ' ');
        });
    }
};
 

//_____________________________________________________________________________________________________________________________

template <  typename StorT, typename SzTypeT = uint32_t, typename AtmT = std::true_type>
struct  Sc_AtmVec : public Cv_FlockSlice< Sc_AtmVec< StorT, SzTypeT, AtmT>, StorT, SzTypeT, AtmT>
{     
    typedef  Cv_FlockSlice< Sc_AtmVec< StorT, SzTypeT, AtmT>, StorT, SzTypeT, AtmT>        Base;

    typedef  SzTypeT                                SzType;
    typedef  StorT                                  Stor;  
    typedef  typename Base::AtmSize                 AtmSize;
    typedef  typename Base::AtmStor                 AtmStor; 
    typedef  Sc_AtmArr< Stor, SzType, AtmT>         AtmArr;  

    AtmSize         m_StkSz;
    AtmArr          m_ArrFleck;
    
public:
    Sc_AtmVec( void)
        : m_StkSz( 0), m_ArrFleck() 
    {} 
    
    ~Sc_AtmVec( void)
    {}
    
    void            DoInit( SzType sz)
    {
        m_StkSz.Set( 0);
        m_ArrFleck.DoInit( sz);  
    }
 
    void            DoIndicize( SzType sz)
    {
        this->m_StkSz.Set( sz);
        this->m_ArrFleck.DoIndicize( sz);  
    }

    AtmStor         *Vec( void) { return m_ArrFleck.Vec(); }
    const AtmStor   *Vec( void) const { return m_ArrFleck.Vec(); }

    AtmSize         *AtmCount( void) { return &m_StkSz; }
    const AtmSize   *AtmCount( void) const { return &m_StkSz; }
 

    SzType          CountMax( void) const { return m_ArrFleck.Size(); }

    SzType          Size( void) const { return m_ArrFleck.Size(); }
    void            Resize( SzType sz)  {  m_ArrFleck.Resize( sz); }
};

//_____________________________________________________________________________________________________________________________
 