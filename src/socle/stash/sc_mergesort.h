// sc_mergesort.h -------------------------------------------------------------------------------------------------------------------
#pragma once
   
//_____________________________________________________________________________________________________________________________

struct Sc_Sort
{  
    //_____________________________________________________________________________________________________________________________

    template < typename InpIt, typename OutIt, typename Pred> 
    struct MergeSorter 
    {
	    typedef	decltype(*std::declval<OutIt>()) 	            DeRef;  
	    typedef typename std::remove_reference< DeRef>::type	Value;  
 
        InpIt	    m_InpIt;
	    OutIt	    m_OutIt;
	    OutIt	    m_AuxIt;
        Pred        m_Pred;
	    uint32_t    m_Size;
	    
	    //_________________________________________________________________________________________________________________________

	    MergeSorter( InpIt inpIt, uint32_t sz, OutIt outIt, OutIt auxIt, const Pred &pred)
		    : m_InpIt( inpIt), m_OutIt( outIt), m_AuxIt( auxIt), m_Pred( pred), m_Size( sz)
	    {}
  
        //_________________________________________________________________________________________________________________________

    template < typename TaskQueue, typename ErrandId = typename TaskQueue::ErrandId>
	    void    operator()( ErrandId succ, TaskQueue *queue) const
	    { 
            //CV_FNTRACE(())
            
            if ( false &&  (  m_Size == 1))
		    {  
                *m_OutIt = *m_InpIt;   
			    return;
		    }

            if ( ( m_Size <= 1024))
		    {  
                auto    ss = StableSort( m_InpIt, m_Size, m_OutIt,m_Pred);   
                ss( succ, queue);
			    return;
		    }
		 
		    uint32_t	split = m_Size /2; 

		    auto	    msMerge = Sc_Sort::Merge( m_OutIt, m_AuxIt, split, m_Size, m_Pred); 
            auto        mergeTask = queue->Construct( succ, msMerge);   
               
            auto	    msLow = Sc_Sort::MergeSort( m_InpIt,  split, m_OutIt,  m_AuxIt, m_Pred); 
            queue->EnqueueTask( queue->Construct( mergeTask, msLow));    

            auto	    msHigh = Sc_Sort::MergeSort( m_InpIt+split, m_Size-split, m_OutIt+split,  m_AuxIt+split, m_Pred);
		    queue->EnqueueTask( queue->Construct( mergeTask, msHigh));  
	    }
 
        //_________________________________________________________________________________________________________________________ 
    };

    //_____________________________________________________________________________________________________________________________

template < typename InpIt, typename OutIt, typename Pred > 
    struct StableSorter 
    {
	    typedef	decltype( *std::declval<OutIt>()) 	            DeRef;  
	    typedef typename std::remove_reference< DeRef>::type	Value;  
 
        Pred        m_Pred;
	    InpIt	    m_InpIt;
	    OutIt	    m_OutIt;
	    uint32_t    m_Size;

	    //_________________________________________________________________________________________________________________________

	    StableSorter( InpIt inpIt, uint32_t sz, OutIt outIt, const Pred &pred)
		    : m_Pred( pred), m_InpIt( inpIt), m_OutIt( outIt), m_Size( sz)
	    {}
 
	    //_________________________________________________________________________________________________________________________

template < typename TaskQueue, typename ErrandId = typename TaskQueue::ErrandId>
	    void    operator()( ErrandId succ, TaskQueue *queue) const
        {
            //CV_FNTRACE(())

            std::copy( m_InpIt, m_InpIt+m_Size, m_OutIt);
            std::stable_sort( m_OutIt, m_OutIt+m_Size, m_Pred);
        }  
    };
 
    
template < typename OutIt, typename Pred > 
	struct Merger 
    {
        typedef	decltype(*std::declval<OutIt>()) 	            DeRef;  
	    typedef typename std::remove_reference< DeRef>::type	Value;  
         
        Pred        m_Pred;
        OutIt       m_DataIt; 
        OutIt       m_AuxIt; 
        uint32_t    m_Split;
        uint32_t    m_Size;
    
        Merger(  OutIt dataIt, OutIt auxIt, uint32_t split, uint32_t sz, const Pred &pred)
            : m_Pred( pred), m_DataIt( dataIt), m_AuxIt( auxIt), m_Split( split), m_Size( sz)
        {}

template < typename TaskQueue, typename ErrandId = typename TaskQueue::ErrandId>
	    void    operator()( ErrandId succ, TaskQueue *queue) const
        {
            //CV_FNTRACE(())

            OutIt   data1End = m_DataIt + m_Split;
            OutIt   data2End = m_DataIt + m_Size;

            OutIt   data1It = m_DataIt;
            OutIt   data2It = data1End;
            OutIt   auxIt = m_AuxIt;

            for ( ; ( data1It < data1End) && ( data2It < data2End); ++auxIt)
                *auxIt = m_Pred( *data1It, *data2It) ? *data1It++ : *data2It++;
            for ( ; ( data1It < data1End); ++auxIt, ++data1It)
                *auxIt = *data1It;
            for ( ; ( data2It < data2End); ++auxIt, ++data2It)
                 *auxIt = *data2It;

            for ( OutIt dataIt = m_DataIt, auxIt = m_AuxIt; dataIt <  data2End; ++dataIt, ++auxIt)
                *dataIt = *auxIt;
             
            return;
        }   
    };

template < typename InpIt, typename OutIt, typename Pred> 
	static auto	MergeSort( InpIt inpIt, uint32_t sz, OutIt outIt, OutIt auxIt, const Pred &pred)
	{
		return MergeSorter( inpIt, sz, outIt, auxIt, pred);	
	}
 
template < typename InpIt, typename OutIt> 
	static auto	MergeSort( InpIt inpIt, uint32_t sz, OutIt outIt, OutIt auxIt)
	{
        typedef	decltype(*std::declval<InpIt>()) 	            DeRef;  
	    typedef typename std::remove_reference< DeRef>::type	Value; 
		return MergeSort( inpIt, sz, outIt, auxIt, std::less< Value>());	
	} 

template < typename OutIt, typename Pred> 
	static auto	Merge(  OutIt dataIt, OutIt auxIt, uint32_t split, uint32_t sz, const Pred &pred)
    {
    	return Merger( dataIt, auxIt, split, sz, pred);	
	}

template < typename InpIt, typename OutIt, typename Pred> 
	static auto	StableSort( InpIt inpIt, uint32_t sz, OutIt outIt, const Pred &pred)
	{
		return StableSorter( inpIt, sz, outIt, pred);	
	}

template < typename InpIt, typename OutIt> 
	static auto	StableSort( InpIt inpIt, uint32_t sz, OutIt outIt)
	{
        typedef	decltype(*std::declval<InpIt>()) 	            DeRef;  
	    typedef typename std::remove_reference< DeRef>::type	Value; 
		return StableSort( inpIt, sz, outIt, std::less< Value>());	
	}
}; 
  
//_____________________________________________________________________________________________________________________________
