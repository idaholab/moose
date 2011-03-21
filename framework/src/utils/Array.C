#include "Array.h"

//#include "ColumnMajorMatrix.h"

// libMesh Includes
#include "vector_value.h"
#include "tensor_value.h"
#include "dense_matrix.h"


template<typename T>
Array<T>::Array() :
  _data(NULL),
  _size(0),
  _allocated_size(0)
{
}

template<typename T>
Array<T>::Array(const unsigned int size) :
  _data(NULL),
  _allocated_size(0)
{
  resize(size);
}

template<typename T>
Array<T>::Array(const unsigned int size, const T & default_value) :
  _data(NULL),
  _allocated_size(0)
{
  resize(size);

  setAllValues(default_value);  
}

template<typename T>
void
Array<T>::release()
{
  if (_data != NULL) 
  {
    delete [] _data;
    _data = NULL;
    _allocated_size = _size = 0;
  }
}


// NOTE: this might eventually go to some other place
// Explicit Instantiations
template class Array<Real>;
template class Array<int>;

template class Array<VectorValue<Real> >;
template class Array<Array<VectorValue<Real> > >;
template class Array<Array<Array<VectorValue<Real> > > >;

template class Array<VectorValue<RealTensor> >;
template class Array<Array<VectorValue<RealTensor> > >;
template class Array<Array<Array<VectorValue<RealTensor> > > >;

template class Array<Array<Real> >;

template class Array<RealTensorValue>;
template class Array<Array<RealTensorValue> >;
template class Array<Array<Array<RealTensorValue> > >;

//template class Array<ColumnMajorMatrix>;
template class Array<Array<Array<Real> > >;

template class Array<std::vector<Real> >;
template class Array<DenseMatrix<Real> >;


