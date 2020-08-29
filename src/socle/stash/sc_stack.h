//  sc_stack.h -------------------------------------------------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------------------------------------------------------------


template <class StackVar>
class  Sc_StackVar 
{
protected:
	StackVar            *m_Below;   // the link to next

public:
	Sc_StackVar( StackVar *b = NULL)  
        : m_Below( b) 
    {}
	
    ~Sc_StackVar( void) 
    { Initialize(); }

	void            Initialize( void) { m_Below = NULL; }
	StackVar        *GetBelow( void) const { return m_Below; }
	void            SetBelow( StackVar *b) { m_Below = b; }

};

//---------------------------------------------------------------------------------------------------------------------------------


template <class StackVar>
class  Sc_Stack 
{
protected:

	StackVar    *m_Top;             // the head of the linked-list

public :

	Sc_Stack( StackVar *tp = NULL) 
		: m_Top( tp) 
	{}

	void        Initialize( void) { m_Top = NULL; }

	StackVar    *Top( void)  const { return m_Top; }
	 
	StackVar    *Bottom( void) const 
	{
		StackVar        *tmp = m_Top;
		while ( tmp && tmp->GetBelow())
			tmp = tmp->GetBelow();
		return tmp;
	}
	 
	StackVar    *Pop( void)
	{
		if ( ! m_Top)
			return NULL;
		StackVar        *tmp = m_Top;
		m_Top = m_Top->GetBelow();
		return tmp;
	}
	 
	void         Push( StackVar *v) 
	{       
		if ( ! v) 
			return;
		v->SetBelow( m_Top); 
		m_Top = v; 
        return;
	}
	 
	void         Append( StackVar *v) 
	{       
		if ( !m_Top)
            Push( v);
		else
        {
            StackVar    *tmp = m_Top;
            for ( ; tmp->GetBelow(); tmp = tmp->GetBelow())
                ;
            tmp->SetBelow( v);
        }
        return;
	}
	 
	void         Remove( StackVar *v)
	{
		if ( ! m_Top)
			return;      

		if ( m_Top == v)    
		{    
			Pop();
			return;
		}
		StackVar    *tmp = m_Top;
		do 
		{
			if ( tmp->GetBelow() == v)
				break;
			tmp = tmp->GetBelow();
		} while( tmp);
		assert( tmp && tmp->GetBelow() && ( tmp->GetBelow() == v));
		tmp->SetBelow( v->GetBelow());
		return;
	}
	 
	int  Transfer(  Sc_Stack< StackVar>  *stk)
	{
		int             i = 0;
		for ( StackVar  *t = NULL; t = stk->Pop(); ++i)
			Push( t);
		return i;
	}

	// Reverse ourselves.
	int Reverse( void)
	{       
		Sc_Stack< StackVar>      tmp;
		int                     n = tmp.Transfer( this);
		m_Top = tmp.m_Top;
		return n;
	}

	int  SzVar(  void)
	{
		int             i = 0;
		for ( StackVar  *t = Top(); (t = t->GetBelow()) ; ++i)
			;
		return i;
	}

	bool    Find( StackVar *s) const
	{
		for ( StackVar    *v = Top(); v; v = v->GetBelow())
			if (v == s)
				return true;
		return false;
	}

	StackVar    *AboveOf( StackVar *s, StackVar *t = NULL) const
	{
        StackVar    *v = t;
        if ( !v)
            v = Top();
		for ( ; v; v = v->GetBelow())
			if (v->GetBelow() == s)
				return v;
		return NULL;
	}


    StackVar *PromoteAfter( StackVar *s, StackVar *beg, StackVar *end = NULL)
	{
        if ( !end)
            end = AboveOf( end, beg);

        assert( end);
        
        StackVar *sPrev = AboveOf( beg, s);
        if ( !sPrev)
            return NULL;
        sPrev->SetBelow( end->GetBelow());     
        end->SetBelow( s->GetBelow());
        s->SetBelow(  beg);
        return sPrev;
    }

    
};
 
//---------------------------------------------------------------------------------------------------------------------------------

 