#include "CoupledNeumannBC.h"

template<>
Parameters valid_params<CoupledNeumannBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

CoupledNeumannBC::CoupledNeumannBC(std::string name,
                   Parameters parameters,
                   std::string var_name,
                   unsigned int boundary_id,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as)
 :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
  _value(_parameters.get<Real>("value")),
  _some_var_val(coupledValFace("some_var"))
{}

Real
CoupledNeumannBC::computeQpResidual()
{
  return -_phi_face[_i][_qp]*_value*_some_var_val[_qp];
}
