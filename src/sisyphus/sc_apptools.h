// sc_apptools.h ------------------------------------------------------------------------------------------------------------------
#pragma once

#include    "socle/stash/sc_stack.h"

//---------------------------------------------------------------------------------------------------------------------------------

struct Sc_AppletItem : public Sc_StackVar< Sc_AppletItem>
{
    typedef int (MainFn)( int argc, char *argv[]);

    const char                      *m_Name;
    std::function< MainFn>          m_MainFn;

    static Sc_Stack< Sc_AppletItem>     s_Singleton;

    Sc_AppletItem( const char *name, const std::function< MainFn> &mainFn)
        : m_Name( name), m_MainFn( mainFn)
    {
        s_Singleton.Push( this);
    }

    static int     RunApplet( const char *name, int argc, char *argv[])
    {
        for ( Sc_AppletItem *app = s_Singleton.Top(); app; app = app->GetBelow())
            if ( strcmp( app->m_Name, name) == 0)
                return app->m_MainFn( argc, argv);
        return -1;
    }
};
    
//---------------------------------------------------------------------------------------------------------------------------------

#define    SC_MAINAPPLET( MainCall)                                                                                             \
                                                                                                                                \
    extern int (MainCall)( int argc, char *argv[]);                                                                             \
    Sc_AppletItem        MainCall##AppletItem( #MainCall, MainCall);                                                            \
                                                                                                                                \
    
//---------------------------------------------------------------------------------------------------------------------------------
