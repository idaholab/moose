#include "CoupledDirichletBC.h"

template<>
Parameters valid_params<CoupledDirichletBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}
