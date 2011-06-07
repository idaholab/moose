#include "SymmTensor.h"

Real
SymmTensor::trace() const
{
  return _xx + _yy + _zz;
}
