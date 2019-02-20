#include "OneDMassFreeBC.h"

registerMooseObject("THMApp", OneDMassFreeBC);

template <>
InputParameters
validParams<OneDMassFreeBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  return params;
}

OneDMassFreeBC::OneDMassFreeBC(const InputParameters & parameters)
  : OneDIntegratedBC(parameters),
    _arhouA_var_number(coupled("arhouA")),
    _arhouA(coupledValue("arhouA"))
{
}

Real
OneDMassFreeBC::computeQpResidual()
{
  return _arhouA[_qp] * _normal * _test[_i][_qp];
}

Real
OneDMassFreeBC::computeQpJacobian()
{
  // d(res)/d(arhoA) = 0.
  return 0.;
}

Real
OneDMassFreeBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _arhouA_var_number)
    // dF/darhouA = phi(j) * nx
    return _phi[_j][_qp] * _normal * _test[_i][_qp];
  else
    // Derivative wrt rho*E (in non-isothermal case) is zero
    return 0.;
}
