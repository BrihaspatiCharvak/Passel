// sc_atmtrail.h -------------------------------------------------------------------------------------------------------------------
#pragma once
 
#include    "socle/coffer/sc_atmunit.h" 

//---------------------------------------------------------------------------------------------------------------------------------

template < typename Stor, typename SzType, typename Atm>  
class Sc_AtmTrail;

//---------------------------------------------------------------------------------------------------------------------------------

template < class Deriv, typename StorT, typename SzTypeT, typename AtmT>
class Sc_AtmIter
{  
public: 
    typedef  StorT                                  Stor; 
    typedef  AtmT                                   Atm; 
    typedef  SzTypeT                                SzType;   
    typedef  Sc_Unit< SzType, Atm>                  AtmSize; 
    typedef  Sc_Unit< Stor, Atm>                    AtmStor; 
    typedef  Sc_AtmTrail< Stor, SzType, Atm>      Trail;
     
    Sc_AtmIter( void)
    {} 

    AtmStor            *Vec( void) { return static_cast< Deriv *>( this)->Vec(); }
    const AtmStor      *Vec( void) const { return static_cast< const Deriv *>( this)->Vec(); }
                    
    Stor                At( SzType ind) const { return Vec()[ ind].Get(); }
    void                SetAt( SzType ind, Stor d) { Vec()[ ind].Set( d); } 

    AtmStor            *AtmAt( SzType ind) { return &Vec()[ ind]; }
    const AtmStor      *AtmAt( SzType ind) const { return &Vec()[ ind]; }

    void        DoSort( SzType b, SzType e)
    {
        // sort and make unique all parity entries.
        auto        *beg = AtmAt( b);
        auto        *end = beg +e -b;
        std::sort( beg, end);  
    }

template < typename Lambda, typename... Args>
    void        DoSort( SzType b, SzType e, const Lambda &lambda, const Args &... args)
    {
        // sort and make unique all parity entries.
        auto        *beg = AtmAt( b);
        auto        *end = beg +e -b;
        std::sort( beg, end, [&]( const AtmStor &a, const AtmStor &b) {
            return lambda( a.Get(), b.Get(), args...);
        });  
    }


template < typename Lambda, typename... Args>    
    SzType      LowerBound( SzType  it, SzType  sz, Stor upIndex, const Lambda &lambda, const Args &... args) const
    {
        auto        *rngBeg = AtmAt( it);
        auto        *rngEnd = rngBeg +sz -it;
        auto        iter = std::lower_bound( rngBeg, rngEnd, AtmStor( upIndex), [&]( const AtmStor &a, const AtmStor &b) {
            return lambda( a.Get(), b.Get(), args...);
        });  
        return it +SzType( iter -rngBeg);
    } 

    SzType      LowerBound( SzType  it, SzType  sz, Stor upIndex) const
    {
         return LowerBound( it, sz, upIndex, std::less< Stor>());
    }

template < typename Lambda, typename... Args>    
    SzType      UpperBound( SzType  it, SzType  sz, Stor upIndex, const Lambda &lambda, const Args &... args) const
    {
        auto        *rngBeg = AtmAt( it);
        auto        *rngEnd = rngBeg +sz -it;
        auto        iter = std::upper_bound( rngBeg, rngEnd, AtmStor( upIndex), [&]( const AtmStor &a, const AtmStor &b) {
            return lambda( a.Get(), b.Get(), args...);
        });  
        return it +SzType( iter -rngBeg);
    }  

    SzType      UpperBound( SzType  it, SzType  sz, Stor upIndex) const
    {
         return UpperBound( it, sz, upIndex, std::less< Stor>());
    }   

template  < typename Sorter>
    bool    SortSanity( SzType sz, const Sorter &sorter = std::less< Stor>()) const 
    {
        for ( SzType i = 1; i < sz; ++i)
            if ( !sorter( At( i -1), At( i)))
                return false;    
        return true;
    }
};
 
//---------------------------------------------------------------------------------------------------------------------------------

template < typename Deriv, typename Stor, typename SzType, typename Atm>  
class Cv_FlockSlice : public  Sc_AtmIter< Deriv, Stor, SzType, Atm>
{     
public: 
   typedef Cv_FlockSlice< Deriv, Stor, SzType, Atm>     This;
   typedef Sc_AtmIter< Deriv, Stor, SzType, Atm>      Base;

    using  typename Base::Stor; 
    using  typename Base::SzType;   
    using  typename Base::AtmSize; 
    using  typename Base::AtmStor;

    Cv_FlockSlice(  void)
    {} 
 
    void            Clear( void)  { AtmCount()->Set( 0); }
 
    AtmSize         *AtmCount( void) { return static_cast< Deriv *>( this)->AtmCount(); }
    const AtmSize   *AtmCount( void) const { return static_cast< const Deriv *>( this)->AtmCount(); }

    SzType          CountMax( void) const { return CV_UINT32_MAX; } 

    SzType          SzMax( void) const { return static_cast< const Deriv *>( this)->CountMax(); }
    SzType          SzStk( void) const { return AtmCount()->Get(); }
    
    Stor            Front( void) const { return  this->At( 0); }
    Stor            Back( void) const { return  this->At( AtmCount()->Get() -1); }

    Stor            Top( void) const { return  Tail(); }
    Stor            Bottom( void) const { return  Front(); }

    Stor            PopBack( void) { return  this->At( AtmCount()->Decr()); }
    
    SzType          PushBack( Stor d)  
    {   
        SC_SANITY_ASSERT( SzStk()  < SzMax())
        SzType  ind = AtmCount()->Incr() -1;
        this->SetAt( ind, d);  
        return ind;
    } 
        
    void            DecrStack( SzType nm) { AtmCount()->Decr( nm); }

template < typename Lambda, typename... Args>
    void            DoSortAll( const Lambda &lambda, const Args &... args)
    {
        static_cast< Deriv *>( this)->DoSort( 0, SzStk(), lambda, args...);
    } 

    void        MakeUnique( void)
    { 
        auto        *beg = AtmAt( 0);
        auto        *end = beg  +SzStk();
        auto        *it = std::unique( beg, end);
        SzType      sz = SzType( it-beg);
        AtmCount()->Set( sz);                      
    } 

template < typename Lambda, typename... Args>   
    void        TraverseAll( const Lambda &lambda, const Args &... args) const 
    { 
        Cv_Aid::ForAll( 0,  SzStk(), lambda, args...); 
    }

template < typename VecFlock>     
    void        TransferTo( VecFlock *arr)                                                           // transfer to input vector
    {
        SzType      szCacheVoid = arr->SzMax() -arr->SzStk();                                       // space in incoming Array
        SzType      szAlloc =  szCacheVoid < SzStk() ? szCacheVoid : SzStk();                       // Qty to be moved. 
        SzType      b = SzStk() -szAlloc;
        for ( SzType i = 0; i < szAlloc; ++i)
            arr->PushBack( this->At( b +i));
        AtmCount()->Set( b);
        return;
    } 

template  < typename Sorter>
    bool    SortSanity( const Sorter &sorter = std::less< Stor>()) const  
    {   
        return Base::SortSanity( SzStk(), sorter); 
    }

    friend std::ostream &operator<<( std::ostream &ostr, const Cv_FlockSlice &arr)
    {
        ostr << "[ ";
        arr.TraverseAll( [&]( SzType sv) {
            ostr << arr.At( sv) << ' ';
        }); 
        ostr << "] ";
        return ostr;
    }
};

//---------------------------------------------------------------------------------------------------------------------------------

template < typename Stor, typename SzType, typename Atm>  
class Sc_AtmTrail : public  Cv_FlockSlice< Sc_AtmTrail< Stor, SzType, Atm>, Stor, SzType, Atm>
{   
    typedef  Cv_FlockSlice< Sc_AtmTrail< Stor, SzType, Atm>, Stor, SzType, Atm> Base;

    using  typename Base::Trail; 
    using  typename Base::Stor; 
    using  typename Base::SzType;   
    using  typename Base::AtmSize; 
    using  typename Base::AtmStor;

protected:
    AtmStor         *m_Vec;
    AtmSize         *m_Size;

public: 
    Sc_AtmTrail( void)
        : m_Vec( NULL), m_Size( NULL) 
    {} 
    
    Sc_AtmTrail( AtmStor *vec, AtmSize *sz)
        : m_Vec( vec), m_Size( sz) 
    {} 
 

    AtmStor             *Vec( void) { return m_Vec; }
    const AtmStor       *Vec( void) const { return m_Vec; }
 
    AtmSize             *AtmCount( void) { return m_Size; }
    const AtmSize       *AtmCount( void) const { return m_Size; }
              
    void                SetSzStk( uint32_t sz) { m_Size->Set( sz); }
    Stor                PopFront( void) 
    { 
        Stor    f = Front();
        Advance( 1);
        return f;
    }

    Sc_AtmTrail		&Advance( SzType k)
    { 
        m_Size->Decr( k);
        m_Vec += k;
        return SELF;
    }      
 
	Sc_AtmTrail     &Shorten( SzType k)
    { 
        m_Size->Decr( k);
        return SELF;
    }  

    //---------------------------------------------------------------------------------------------------------------------------------

template < typename LessFn >
    struct InPlaceMerger
    { 
        Trail           *m_Res;                                             // Res and First queues share the data-segment
        Trail           m_FSeq;                                             // first queue
        Trail           m_SSeq;                                             // second queue
        Trail           m_TSeq;                                             // temp stack => The entries are in reverse sorted order
        LessFn          m_Less;                                             // the Lessfn
        AtmSize         m_SzFirst;                                          // hold the size for first queue
        AtmSize         m_SzTemp;                       
        bool            m_EqPrefFirst;

        InPlaceMerger( Trail *fSeq,  const Trail &sSeq, AtmStor *tBuf,  const LessFn &less, bool eqPrefFirst)
            : m_Res( fSeq), m_FSeq( fSeq->Vec(), &m_SzFirst), m_SSeq( sSeq), m_TSeq( tBuf, &m_SzTemp), 
                m_Less( less), m_SzFirst( fSeq->SzStk()), m_SzTemp( 0), m_EqPrefFirst( eqPrefFirst)
        {
            CV_SANITY_ASSERT( fSeq->SortSanity( m_Less))
            CV_SANITY_ASSERT( sSeq.SortSanity( m_Less))
            m_Res->Clear();
        }

        bool    MergeStep( void)
        {
            if ( !m_SSeq.SzStk())                                           // second queue is over
            {
                m_FSeq.TransferTo( &m_TSeq);                                  
                m_TSeq.TransferTo( m_Res);
                return true;
            }
            if ( !m_TSeq.SzStk())
            {   
                CV_SANITY_ASSERT( !m_FSeq.SzStk())
                m_SSeq.TransferTo( m_Res);
                return true;
            }

            auto    fTop = m_TSeq.Front();                                  
            auto    sTop = m_SSeq.Front();                                  // head of second => sTop
            if ( m_Less( fTop, sTop))                                       // fTop strictly less than sTop 
                m_Res->PushBack( m_TSeq.PopFront()); 
            else if ( m_Less( sTop, fTop))                                  // sTop strictly less than fTop 
                m_Res->PushBack( m_SSeq.PopFront());  
            else {
                if ( !m_EqPrefFirst)
                    std::swap( fTop, sTop);
                m_Res->PushBack( fTop); 
                m_Res->PushBack( sTop); 
                m_SSeq.PopFront();
                m_TSeq.PopFront();
            }                
            if ( m_FSeq.SzStk())
                m_TSeq.PushBack( m_FSeq.PopFront());
            return false;
        }
        
        bool    DoProcess( void)
        {
            if ( m_FSeq.SzStk())                                           
                m_TSeq.PushBack( m_FSeq.PopFront());                         // pop the top and push in temp.

            bool          res = false;
            for ( ; !res; )
                res = MergeStep(); 
            CV_SANITY_ASSERT( m_Res->SortSanity( m_Less))
            return res;
        } 
    };
 
    
template < typename LessFn >   
    void    MergeInPlace( const Sc_AtmTrail &sSeq, AtmStor *tBuf, bool eqPrefFirst, const LessFn &less)
    {
        static uint32_t     s_Count = 0;
        ++s_Count;
        uint32_t    fSz = SzStk();
        uint32_t    sSz = sSeq.SzStk();
        
        //if (( fSz > 1000) && ( sSz > 1000))
        //    std::cout << "MergeInPlace: " << fSz << " " << sSz << "\n";
        InPlaceMerger< LessFn>  ipm( this, sSeq, tBuf, less, eqPrefFirst);
        ipm.DoProcess();            
        uint32_t    rSz = SzStk();
        CV_SANITY_ASSERT(( rSz == ( fSz +sSz)) && !sSeq.SzStk())
    }

};
 
//---------------------------------------------------------------------------------------------------------------------------------
