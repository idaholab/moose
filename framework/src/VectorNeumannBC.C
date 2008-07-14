#include "VectorNeumannBC.h"

template<>
Parameters valid_params<VectorNeumannBC>()
{
  Parameters params;
  params.set<Real>("value0")=0.0;
  params.set<Real>("value1")=0.0;
  params.set<Real>("value2")=0.0;
  return params;
}
