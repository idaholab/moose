#include "MatchedValueBC.h"

template<>
InputParameters validParams<MatchedValueBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredCoupledVar("v", "");
  params.set<bool>("_integrated") = false;
  return params;
}

MatchedValueBC::MatchedValueBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _v_face(coupledVal("v"))
  {}

Real
MatchedValueBC::computeQpResidual()
  {
    return _u[_qp]-_v_face[_qp];
  }
