#include "MooseArray.h"

#include "ColumnMajorMatrix.h"

template<typename T>
MooseArray<T>::MooseArray()
  :_allocated_size(0),
   _size(0),
   _data(NULL)
{
}

template<typename T>
MooseArray<T>::MooseArray(const unsigned int size)
  :_allocated_size(0),
   _data(NULL)
{
  resize(size);
}

template<typename T>
void
MooseArray<T>::release()
{
  if (_data != NULL) 
  {
    delete [] _data;
    _data = NULL;
    _allocated_size = _size = 0;
  }
}

// Explicit Instantiations
template class MooseArray<Real>;
template class MooseArray<RealGradient>;
template class MooseArray<MooseArray<Real> >;
template class MooseArray<RealTensorValue>;
template class MooseArray<ColumnMajorMatrix>;
template class MooseArray<MooseArray<MooseArray<Real> > >;
