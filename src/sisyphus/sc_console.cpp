// sc_console.cpp -----------------------------------------------------------------------------------------------------------------

#include    "socle/tenor/sc_include.h"
#include    "sisyphus/sc_apptools.h"
#include    "socle/stash/sc_value.h" 
#include    "socle/stash/sc_mergesort.h" 
#include    "socle/coffer/sc_atmunit.h"
#include    "socle/coffer/sc_atmtrail.h" 
#include    "socle/coffer/sc_atmarray.h" 
#include    "socle/coffer/sc_freestore.h" 
#include    "socle/pinion/sc_taskscheduler.h" 

//---------------------------------------------------------------------------------------------------------------------------------

Sc_Stack< Sc_AppletItem>     Sc_AppletItem::s_Singleton;

SC_MAINAPPLET( testapp)
SC_MAINAPPLET( mergeapp)

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
    constexpr  uint32_t     Sz = sizeof( input)/ sizeof( Value); 

    Value		            output[ Sz]; 
    Value		            aux[ Sz]; 
    Sc_TaskScheduler        scheduler( 4);
    scheduler.DoInit();
    {
        Sc_TaskSession      *queue = scheduler.CurSession(); 
 
        auto	            ms = Sc_Sort::MergeSort(  input, Sz, output, aux, Value::Less()); 
        queue->EnqueueTask( queue->Construct( Sc_TaskScheduler::NullTask(), ms));   
    
        scheduler.DoLaunch();
    }
    for (int i = 0; i < Sz; i++)
    {
        std::cout << *(output + i) << "\n";
    } 
    return 0;
}
 
 
//---------------------------------------------------------------------------------------------------------------------------------
