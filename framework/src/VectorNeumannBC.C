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

VectorNeumannBC::VectorNeumannBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as)
  {
    _value(0)=_parameters.get<Real>("value0");
    _value(1)=_parameters.get<Real>("value1");
    _value(2)=_parameters.get<Real>("value2");
  }


Real
VectorNeumannBC::computeQpResidual()
  {
    return -_phi_face[_i][_qp]*(_value*_normals_face[_qp]);
  }

