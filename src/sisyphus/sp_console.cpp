// sc_console.cpp -----------------------------------------------------------------------------------------------------------------

#include    "socle/tenor/sc_include.h"
#include    "sisyphus/sp_apptools.h"
#include    "socle/coffer/sc_atmunit.h"
#include    "socle/coffer/sc_atmtrail.h" 
#include    "socle/coffer/sc_atmarray.h" 
#include    "socle/coffer/sc_freestore.h" 

//---------------------------------------------------------------------------------------------------------------------------------

Sc_Stack< Sc_AppletItem>     Sc_AppletItem::s_Singleton;

SC_MAINAPPLET( testapp)

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
    Sc_AtmTrail< uint32_t, uint32_t, std::true_type>  trail;
    Sc_TaskScheduler                        scheduler( 4);

    return 0;
}
 
//---------------------------------------------------------------------------------------------------------------------------------
