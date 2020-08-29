// sc_freestore.h ------------------------------------------------------------------------------------------------------------------- 
#pragma once 

#include    "socle/coffer/sc_atmunit.h"
#include    "socle/coffer/sc_atmarray.h"

//---------------------------------------------------------------------------------------------------------------------------------
// Memory dispensor of Type X, IndexType DStor, of Max ppol-size Mx

template < typename DerivedStore,  class X, typename StorT, uint32_t Mx>
struct Sc_BaseStore
{   
     
    typedef std::true_type                      Atomic;
    typedef StorT                               Stor;
    typedef X                                   Type; 
    typedef Sc_AtmVec<  Stor>                   ObjStack;                               
                                 
    ObjStack                        m_Store;                                // Stack of free Indexes into Page at each object location
    Sc_Spinlock<Atomic>             m_Lock;                                 // SpinLock to be used for concurrent access   
    uint32_t                        m_StoreId;
    
    DerivedStore                    *FetchStore( void) const { return ( DerivedStore *) this; }
    uint8_t                         *Page( void) const { return FetchStore()->Page(); }

    Sc_BaseStore( uint32_t storeId)
        :   m_StoreId( storeId)
    {       
        m_Store.DoIndicize( Mx);                                            // initially, all are free and indexes by integer-sequence        
    }
    
    ~Sc_BaseStore( void)
    {}
     
    
template < typename CacheStore>
    void        AllocBulk( CacheStore *cacheStore)                          // allocate in bulk : caller needs to SpinLock if needed
    {
       m_Store.TransferTo( cacheStore);  
    } 

template < typename CacheStore>
    void        DiscardBulk( CacheStore *cacheStore)                        // discard in bulk : caller needs to SpinLock if needed
    {
        cacheStore->TransferTo( &m_Store); 
    }
 
    Type        *Alloc( void)                                               // allocaate one : caller needs to SpinLock if needed
    { 
        return m_Store.SzStk() ? FetchStore()->GetAt( m_Store.Pop()) : NULL;
    }
    
    void        Discard( Type *x)                                           // discard one : caller needs to SpinLock if needed
    { 
        m_Store.Push( FetchStore()->MapId( x)); 
    } 
};

//---------------------------------------------------------------------------------------------------------------------------------
// Memory dispensor of Type X, IndexType DStor, of Max ppol-size Mx

template < class X, typename DStor, uint32_t Mx, uint32_t ObjSz = sizeof( X)>
struct Sc_FreeStore : public Sc_BaseStore< Sc_FreeStore< X, DStor, Mx,  ObjSz>, X, DStor, Mx>
{   
    typedef Sc_BaseStore< Sc_FreeStore< X, DStor, Mx,  ObjSz>, X, DStor, Mx>    Base;
    
    uint8_t                         m_Page[ Mx * ObjSz]; 
    
    Sc_FreeStore( uint32_t storeId)
        :  Base( storeId)
    {} 
    
    X           *GetAt( DStor id) const                              // get pointer for index
    { 
        SC_SANITY_ASSERT( id < Mx)
        return reinterpret_cast< X *>( const_cast< uint8_t *>( &m_Page[ id * ObjSz])); 
    }
    
    DStor        MapId( const X *x) const                                // get index for pointer
    {   
        uint64_t        id( ( reinterpret_cast< const uint8_t *>( x) - m_Page) /ObjSz);
        SC_SANITY_ASSERT( id < Mx)
        return DStor( id); 
    } 
};
 
//---------------------------------------------------------------------------------------------------------------------------------

// temporary cache at each CPU so that only bulk ops are carried out with SpinLocks.

template < uint32_t CacheSz, typename Store>
struct Sc_FreeCache
{ 
    typedef  typename Store::Type                   Type;
    typedef  typename Store::Stor                   Stor;
    typedef  typename Store::Atomic                 Atomic;
    typedef  typename Sc_Spinlock<Atomic>::Guard    Guard;
    typedef Sc_AtmVec< Stor>                        CacheStack;
    
    Store                       *m_FreeStore;
    CacheStack                  m_CacheStore;
//    std::vector< bool>        m_Bits;
    
    Sc_FreeCache( void)
       : m_FreeStore( NULL)
        //, m_Bits( Store::StorMax, false)
    {
        m_CacheStore.DoInit( CacheSz);
    } 
    
    Store           *GetStore( void) const { return m_FreeStore; }
    void            SetStore( Store *store) { m_FreeStore = store; }

    uint8_t         StoreId( void) const { return  m_FreeStore->m_StoreId; }
    uint32_t        SzFree( void)  const { return  m_CacheStore.SzStk(); }
        
    uint32_t        ProbeSzFree( uint32_t szExpect = 1)                 //  The number of free entries in Cache, if none try fetch.
    { 
        if ( m_CacheStore.SzStk() >= szExpect)
            return  m_CacheStore.SzStk(); 
        {
            Guard      guard( &m_FreeStore->m_Lock); 
            m_FreeStore->AllocBulk( &m_CacheStore); 
        }
        return  m_CacheStore.SzStk();
    }
    
    uint32_t    AllocBulk( Type **xArr, uint32_t sz)                    // allocate from cache and return pointers to free location
    {        
        uint32_t    szAlloc =  sz < m_CacheStore.SzStk() ? sz : m_CacheStore.SzStk();
        for ( uint32_t i = 0; i < szAlloc; ++i)
        {
            xArr[ i] = m_FreeStore->GetAt( m_CacheStore[ m_CacheStore.SzStk() -szAlloc +i ]);
            //CV_PREFETCH_CACHE( xArr[ i])
            *(( uint64_t *) xArr[ i]) = 0;
        }
        m_CacheStore.DecrFill( szAlloc);
        return szAlloc;
    }
     
    Stor    AllocId( void)                                        // return pointer to free location
    { 
        return m_CacheStore.PopBack();
    } 

    Type    *GetAtFromId( Stor id) const   { return m_FreeStore->GetAt( id);  }


    Type    *AllocFree( void)                                        // return pointer to free location
    { 
        Stor    id = AllocId();
//        CV_ERROR_ASSERT( !m_Bits[ id] && (( m_Bits[ id] = true)))
        Type    *x = m_FreeStore->GetAt( id); 
        SC_SANITY_ASSERT( x->SetStoreId( m_FreeStore->m_StoreId))
        return  x; 
    } 
    
    void    Discard( Stor id)                                           // discard object at Id
    { 
//        CV_ERROR_ASSERT( m_Bits[ id] && (!( m_Bits[ id] = false)))
        if ( m_CacheStore.SzStk() == m_CacheStore.Size())
        {
            Guard      guard( &m_FreeStore->m_Lock);                    // setup an SpinLock guard and return to buffer to  Store if overflow
            m_FreeStore->DiscardBulk( &m_CacheStore); 
        }
        m_CacheStore.PushBack( id);
    }
    
    void    Discard( void *x) 
    {
        SC_SANITY_ASSERT( x->StoreId() == m_FreeStore->m_StoreId)
        //memset( x, 0xCC, Store::ObjSz);
        Discard( m_FreeStore->MapId( x)); 
    }    // Discard an object
}; 

//---------------------------------------------------------------------------------------------------------------------------------

