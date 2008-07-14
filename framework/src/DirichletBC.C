#include "DirichletBC.h"

template<>
Parameters valid_params<DirichletBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}
