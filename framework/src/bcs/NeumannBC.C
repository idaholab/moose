#include "NeumannBC.h"

template<>
InputParameters valid_params<NeumannBC>()
{
  InputParameters params;
  params.set<Real>("value")=0.0;
  return params;
}

NeumannBC::NeumannBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value"))
 {}

Real
NeumannBC::computeQpResidual()
  {
    return -_phi_face[_i][_qp]*_value;
  }

