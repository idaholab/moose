#include "CHCpldPFCTrad.h"

template<>
InputParameters validParams<CHCpldPFCTrad>()
{
  InputParameters params = validParams<CHSplitVar>();
  params.addRequiredParam<std::string>("coeff_name", "Name of coefficient");
  return params;
}

CHCpldPFCTrad::CHCpldPFCTrad(const std::string & name,
                             InputParameters parameters) :
    CHSplitVar(name, parameters),
    _coeff_name(getParam<std::string>("coeff_name")),
    _coeff(getMaterialProperty<Real>(_coeff_name))
{
}

RealGradient
CHCpldPFCTrad::precomputeQpResidual()
{
  RealGradient grad_cpldvar = CHSplitVar::precomputeQpResidual();
  return  _coeff[_qp]*grad_cpldvar;
}

Real
CHCpldPFCTrad::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real grphi_grtst = CHSplitVar::computeQpOffDiagJacobian(jvar);
  return _coeff[_qp]*grphi_grtst;
}
