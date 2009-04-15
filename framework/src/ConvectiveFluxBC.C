#include "ConvectiveFluxBC.h"

template<>
Parameters valid_params<ConvectiveFluxBC>()
{
  Parameters params;
  params.set<Real>("rate")=7500;
  params.set<Real>("value")=500;
  return params;
}

Real ConvectiveFluxBC::computeQpResidual()
{
  return -(_phi_face[_i][_qp]*_rate*(_u_face[_qp] - _value));
}
  
Real ConvectiveFluxBC::computeQpJacobian()
{
  return -(_phi_face[_i][_qp]*_rate*(_phi_face[_j][_qp] - _value));
}
