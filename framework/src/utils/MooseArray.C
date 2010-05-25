#include "MooseArray.h"

#include "ColumnMajorMatrix.h"

// libMesh Includes
#include "vector_value.h"

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
MooseArray<T>::MooseArray(const unsigned int size, const T & default_value)
  :_allocated_size(0),
   _data(NULL)
{
  resize(size);

  setAllValues(default_value);  
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

template class MooseArray<VectorValue<Real> >;
template class MooseArray<MooseArray<VectorValue<Real> > >;
template class MooseArray<MooseArray<MooseArray<VectorValue<Real> > > >;

template class MooseArray<VectorValue<RealTensor> >;
template class MooseArray<MooseArray<VectorValue<RealTensor> > >;
template class MooseArray<MooseArray<MooseArray<VectorValue<RealTensor> > > >;

template class MooseArray<MooseArray<Real> >;

template class MooseArray<RealTensorValue>;
template class MooseArray<MooseArray<RealTensorValue> >;
template class MooseArray<MooseArray<MooseArray<RealTensorValue> > >;

template class MooseArray<ColumnMajorMatrix>;
template class MooseArray<MooseArray<MooseArray<Real> > >;
