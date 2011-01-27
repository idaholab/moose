#include "ElasticityTensor.h"

ElasticityTensor::ElasticityTensor(const bool constant)
  : ColumnMajorMatrix(9,9),
    _constant(constant),
    _values_computed(false)
{}

void ElasticityTensor::calculate(unsigned int qp)
{  
  if(!_constant || !_values_computed)
  {
    calculateEntries(qp);
    _values_computed = true;
  }
}

ColumnMajorMatrix
ElasticityTensor::calculateDerivative(unsigned int /*qp*/, unsigned int /*i*/)
{
  ColumnMajorMatrix m(9, 9);
  return m;
}


