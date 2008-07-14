#include "NeumannBC.h"

template<>
Parameters valid_params<NeumannBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}
