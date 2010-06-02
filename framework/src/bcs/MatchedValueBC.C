#include "MatchedValueBC.h"

template<>
InputParameters validParams<MatchedValueBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredCoupledVar("v", "");
  return params;
}

MatchedValueBC::MatchedValueBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, false)),
    _v_face(coupledValFace("v"))
  {}

Real
MatchedValueBC::computeQpResidual()
  {
    return _u_face[_qp]-_v_face[_qp];
  }
