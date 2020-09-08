// sc_splitsort.h -------------------------------------------------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------------------
// Stable, non-recursive mergesort algorithm. Sort a[:n], using temporary space of size n starting at tmp. 

template <typename T, typename IsLess, typename SzType = size_t>
class Sc_SplitSort
{
public:
    void MergeSort( T *a, SzType n, T* tmp, IsLess is_less) 
    {
        for (SzType f = 1; f < n; f += 2)                           // Unfold the first pass for speedup.
        { 
            if (is_less(a[f], a[f - 1])) 
            {
                T   t = a[f];
                a[ f] = a[f - 1];
                a[ f - 1] = t;
            }
        }
        bool    s = false;
        for ( SzType p = 2; p != 0 && p < n; p <<= 1, s = !s) 
        {
            // Now all sublists of p are already sorted.
            T       *z = tmp;
            for ( SzType i = 0; i < n; i += p << 1) 
            {
                T       *x = a + i;
                T       *y = x + p;
                SzType  xn = p < n - i ? p : n - i;
                SzType  yn = (p << 1) < n - i ? p : p < n - i ? n - p - i : 0;
                if (xn > 0 && yn > 0 && is_less( *y, x[xn - 1]))    // Optimization (S), Java 1.6 also has it.
                {  
                    for (;;) 
                    {
                        if ( is_less(*y, *x)) 
                        {
                            *z++ = *y++;
                            if (--yn == 0) 
                                break;
                        }
                        else 
                        {
                            *z++ = *x++;
                            if (--xn == 0) 
                                break;
                        }
                    }
                }
                while (xn > 0)                                      // Copy from *x first because of (S).
                {
                    *z++ = *x++;
                    --xn;
                }
                while (yn > 0) 
                {
                    *z++ = *y++;
                    --yn;
                }
            }
            z = a; a = tmp; tmp = z;
        }
        if (s)                                                      // Copy from tmp to result.
        {
            for (T* x = tmp, *y = a, * const x_end = tmp + n; x != x_end; ++x, ++y) 
                *x = *y;            
        }
    }

    // Stable, non-recursive MergeSort.
    // To sort vector `a', call MergeSort(a.data(), a.data() + a.size(), is_less)'
    // or use the convenience function below.
template <typename T, typename IsLess>
    void MergeSort( T* a, T *a_end, IsLess is_less) 
    {
        const SzType        n = a_end - a;
        if (n < 2) 
            return;
        // Creating ptr_deleter so tmp will be deleted even if is_less or
        // MergeSort throws an exception.
        struct ptr_deleter 
        {
            T* p_;

            ptr_deleter(T* p) 
              : p_(p) 
            {}
            ~ptr_deleter() 
            { 
                delete p_; 
            }
        } tmp( new T[n]);
        MergeSort(a, n, tmp.p_, is_less);
    }

    // Convenience function to sort a range in a vector.
template <typename T, typename IsLess>
    void MergeSort(const T& begin, const T& end, IsLess is_less) 
    {
        MergeSort(&*begin, &*end, is_less);
    }

    // Stable, non-recursive MergeSort.
    // Resizes v to double size temporarily, and then changes it back. Memory may
    // be wasted in b after the call because of that.
template <typename T, typename IsLess>
    void mergesort_consecutive(std::vector<T>* v, IsLess is_less) 
    {
        const SzType n = v->size();
        if (n < 2) 
            return;
        v->resize(n << 1);  // Allocate temporary space.
        MergeSort(v->data(), n, v->data() + n, is_less);
        v->resize(n);
    }
};
