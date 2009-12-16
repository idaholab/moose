#include "ElasticityTensor.h"

ElasticityTensor::ElasticityTensor(const bool constant)
  : ColumnMajorMatrix(9,9),
    _constant(constant),
    _values_computed(false)
{}

void ElasticityTensor::calculate()
{  
  if(!_constant || !_values_computed)
  {
    calculateEntries();
    _values_computed = true;
  }
}



