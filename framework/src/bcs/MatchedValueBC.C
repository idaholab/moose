#include "MatchedValueBC.h"

template<>
Parameters valid_params<MatchedValueBC>()
{
  Parameters params;
  return params;
}

MatchedValueBC::MatchedValueBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _v_face(coupledValFace("v"))
  {}

Real
MatchedValueBC::computeQpResidual()
  {
    return _u_face[_qp]-_v_face[_qp];
  }
