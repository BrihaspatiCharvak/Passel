// sc_console.cpp -----------------------------------------------------------------------------------------------------------------

#include    "socle/tenor/sc_include.h"
#include    "sisyphus/sc_apptools.h"
#include    "socle/stash/sc_value.h" 
#include    "socle/stash/sc_splitsort.h" 
#include    "socle/stash/sc_order.h"
#include    "socle/coffer/sc_atmunit.h"
#include    "socle/coffer/sc_atmtrail.h" 
#include    "socle/coffer/sc_atmarray.h" 
#include    "socle/coffer/sc_freestore.h" 
#include    "socle/pinion/sc_taskscheduler.h" 

//---------------------------------------------------------------------------------------------------------------------------------

Sc_Stack< Sc_AppletItem>     Sc_AppletItem::s_Singleton;

SC_MAINAPPLET( testapp)
SC_MAINAPPLET( mergeapp)
SC_MAINAPPLET( sortapp)

//---------------------------------------------------------------------------------------------------------------------------------

int main( int argc, char *argv[])
{
    if ( argc < 2)
    {
        std::cout << "No Applet Arg\n";
        return -1;
    }
    return Sc_AppletItem::RunApplet( argv[ 1], argc -2, argv +2);
}

//---------------------------------------------------------------------------------------------------------------------------------

int testapp( int argc, char *argv[])
{
    Sc_AtmArr<uint32_t>                                 arr;
    Sc_AtmVec<uint32_t>                                 vec;
    vec.PushBack( 20);
    uint32_t    t = vec.PopBack();
    Sc_AtmTrail< uint32_t, uint32_t, std::true_type>  trail;
  //  Sc_TaskScheduler                        scheduler( 4);

    return 0;
}
 
//---------------------------------------------------------------------------------------------------------------------------------

int mergeapp( int argc, char *argv[])
{ 
    typedef Sc_Value< int32_t>      Value;
    
    Value	                input[] = {5,10,15,20,25,50,40,30,20,10,9524,878,17,1,-99,18785,3649,-3,164,94};
    constexpr uint32_t      Sz = sizeof( input)/ sizeof( Value); 

    Value		            output[ Sz]; 
    Value		            aux[ Sz]; 
    Sc_TaskScheduler        scheduler( 4);
    scheduler.DoInit();
    {
        Sc_TaskSession      *queue = scheduler.CurSession(); 
 
        auto	            ms = Sc_Order::SplitSort(  input, Sz, output, aux); 
        queue->EnqueueTask( queue->Construct( Sc_TaskScheduler::NullTask(), ms));   
    
        scheduler.DoLaunch();
    }
    for (int i = 0; i < Sz; i++) 
        std::cout << *(output + i) << "\n"; 
    std::cout << Sc_MSort::LowerBound( output, uint32_t( 0), Sz, Value( 25)) << "\n"; 
    std::cout << Sc_MSort::LowerBound( output, uint32_t( 0), Sz, Value( 164)) << "\n"; 
    std::cout << Sc_MSort::LowerBound( output, uint32_t( 0), Sz, Value( 16)) << "\n"; 
    return 0;
}
 
//---------------------------------------------------------------------------------------------------------------------------------

int sorter1( int argc, char *argv[])
{ 
    int arr[] = {3, 1, 23, -9, 233, 23, -313, 32, -9}; 
    int n = sizeof(arr) / sizeof(arr[0]); 

    // Pass the array, the pointer to the first element and 
    //// the pointer to the last element 
    //Introsort(arr, arr, arr+n-1); 
    //printArray(arr, n); 

    return(0); 
} 

//---------------------------------------------------------------------------------------------------------------------------------

int sortapp( int argc, char *argv[])
{ 
    typedef int32_t         Value;

    Value	                input[] = {20,25,5,10,15,6};
    uint32_t                sz = sizeof( input)/ sizeof( Value); 
    
    //Sc_Order::InsertionSort( input, uint32_t( 0), sz);
    //uint32_t        low = Sc_Order::LowerBound( input, 0, sz, 25);
    //std::cout << low << "\n";

    for ( uint32_t i = 0; i < sz; i++)
        std::cout << *(input + i) << "\n";
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------
