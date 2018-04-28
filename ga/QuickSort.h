// QuickSort.h Template-Version

// need to provide a GreaterThan(T& x, T& y) function
// that returns true when x > y and false otherwise

#ifndef QUICKSORT_H
#define QUICKSORT_H

template <class T>                       // swap template
inline void Swap(T& x, T& y)
{
  T tmp = x;
  x = y; 
  y = tmp;
}

template <class T>                      // Quicksort-Template
void QuickSort(T v[], int n) 
{                                       // cf. K&R, p.87
  if(n <= 1) return;
  int last = 0;
  for (int i = 1; i < n; i++)
    if (GreaterThan(v[0], v[i]))
      Swap(v[++last], v[i]);
  Swap(v[0],v[last]);
  QuickSort(v, last); 
  QuickSort(v+last+1, n-last-1);
}

#endif // QUICKSORT_H
