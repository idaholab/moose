/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseArray.h"
#include "ColumnMajorMatrix.h"
#include "SymmTensor.h"
// libMesh Includes
#include "vector_value.h"
#include "tensor_value.h"
#include "dense_matrix.h"


template<typename T>
MooseArray<T>::MooseArray() :
  _data(NULL),
  _size(0),
  _allocated_size(0)
{
}

template<typename T>
MooseArray<T>::MooseArray(const unsigned int size) :
  _data(NULL),
  _allocated_size(0)
{
  resize(size);
}

template<typename T>
MooseArray<T>::MooseArray(const unsigned int size, const T & default_value) :
  _data(NULL),
  _allocated_size(0)
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


// NOTE: this might eventually go to some other place
// Explicit Instantiations
template class MooseArray<Real>;
template class MooseArray<int>;

template class MooseArray<VectorValue<Real> >;
template class MooseArray<MooseArray<VectorValue<Real> > >;
template class MooseArray<MooseArray<MooseArray<VectorValue<Real> > > >;

template class MooseArray<MooseArray<VectorValue<RealTensor> > >;
template class MooseArray<MooseArray<MooseArray<VectorValue<RealTensor> > > >;

template class MooseArray<MooseArray<Real> >;

template class MooseArray<RealTensorValue>;
template class MooseArray<MooseArray<RealTensorValue> >;
template class MooseArray<MooseArray<MooseArray<RealTensorValue> > >;

template class MooseArray<ColumnMajorMatrix>;
template class MooseArray<MooseArray<MooseArray<Real> > >;

template class MooseArray<std::vector<Real> >;
template class MooseArray<DenseMatrix<Real> >;

template class MooseArray<SymmTensor>;
