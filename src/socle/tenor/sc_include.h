// sc_include.h -------------------------------------------------------------------------------------------------------------------

#pragma once 

#define _CRT_SECURE_NO_WARNINGS

#if defined(__GNUC__) || defined(__clang__)
    #define DISABLE_WARNING_PUSH                __pragma(GCC diagnostic push)
    #define DISABLE_WARNING_POP                 __pragma(GCC diagnostic pop) 
    #define GCC_DISABLE_WARNING(warningName)    __pragma(GCC diagnostic ignored #warningName)

    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wunused-function"
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wmicrosoft-template"
    #pragma GCC diagnostic ignored "-Winconsistent-missing-override"
    #pragma GCC diagnostic ignored "-Wshift-negative-value"
    
#elif defined(_MSC_VER)

    #define DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define DISABLE_WARNING_POP            __pragma(warning( pop )) 

    #pragma warning(disable: 4100)
    #pragma warning( disable :4355)
    #pragma warning( disable :4996)

#endif

//---------------------------------------------------------------------------------------------------------------------------------

#include    <cstdlib>
#include    <cmath>
#include    <stdexcept>
#include    <memory>
#include    <numeric>
#include    <limits>
#include    <functional>
#include    <algorithm>
#include	<utility>
#include    <string>
#include    <typeinfo>
#include	<tuple>
#include	<iterator>
#include    <array>
#include    <vector>
#include    <bitset>
#include    <iostream>
#include    <fstream>
#include    <sstream>
#include    <chrono>
#include    <ctime>
#include    <random>
#include    <atomic>
#include    <thread>
#include    <mutex>

//---------------------------------------------------------------------------------------------------------------------------------

struct Sc_ErrorCntl
{

    bool ErrorTrap( const char *file, uint32_t line);
};

 
//---------------------------------------------------------------------------------------------------------------------------------
  