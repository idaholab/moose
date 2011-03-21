#include "MatchedValueBC.h"

template<>
InputParameters validParams<MatchedValueBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredCoupledVar("v", "");
  return params;
}

MatchedValueBC::MatchedValueBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
    _v_face(coupledValue("v"))
{
}

Real
MatchedValueBC::computeQpResidual()
{
  return _u[_qp]-_v_face[_qp];
}
