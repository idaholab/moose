//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticityTensor.h"

ElasticityTensor::ElasticityTensor(const bool constant)
  : ColumnMajorMatrix(9, 9), _constant(constant), _values_computed(false)
{
}

void
ElasticityTensor::calculate(unsigned int qp)
{
  if (!_constant || !_values_computed)
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
