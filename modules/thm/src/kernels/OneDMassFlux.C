#include "OneDMassFlux.h"

template<>
InputParameters validParams<OneDMassFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rhouA", "conserved x-momentum");

  return params;
}

OneDMassFlux::OneDMassFlux(const InputParameters & parameters) :
    Kernel(parameters),
    _rhouA(coupledValue("rhouA")),
    _rhouA_var_number(coupled("rhouA"))
{
}

OneDMassFlux::~OneDMassFlux()
{
}

Real
OneDMassFlux::computeQpResidual()
{
  // Note: negative sign comes from integration by parts
  return -_rhouA[_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMassFlux::computeQpJacobian()
{
  // This seems weird at first glance, but this term does not depend
  // on U0, only the conserved variable U1.  In other words the (1,1)
  // entry of the flux Jacobian is 0.
  return 0.;
}

Real
OneDMassFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhouA_var_number)
    return -_phi[_j][_qp] * _grad_test[_i][_qp](0);
  else
    return 0.;
}
