// sc_value.h ---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------------------

template <typename Type>
struct  Sc_Value
{
    Type     m_Value;

public:
    Sc_Value( Type v = 0)
        : m_Value( v)
    {} 

    const Type  &Value( void) const { return m_Value; }
    void        SetValue( const Type  &t) { m_Value = t; }

    friend std::ostream &operator<<( std::ostream &ostr, const Sc_Value &t)
    {
        ostr << t.m_Value;
        return ostr;
    }

    bool    operator<( const Sc_Value &t2) const
    { 
        return m_Value < t2.m_Value;
    }
    
};

//---------------------------------------------------------------------------------------------------------------------------------
