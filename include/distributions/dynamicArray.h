#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

template <typename T>
T **
allocateDynamicArray(int n_rows, int n_cols)
{
  T **dynamicArray;

  dynamicArray = new T*[n_rows];
  for ( int i = 0 ; i < n_rows ; i++ )
  {
    dynamicArray[i] = new T [n_cols];
  }

  return dynamicArray;
}

template <typename T>
void
freeDynamicArray(T** d_array)
{
  delete [] *d_array;
  delete [] d_array;
}

#endif /* DYNAMICARRAY_H */
