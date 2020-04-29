/// Considers cleavage plane anisotropy in the crack propagation

#include "ACInterfaceCleavageFracture.h"

registerMooseObject("PhaseFieldApp", ACInterfaceCleavageFracture);

InputParameters
ACInterfaceCleavageFracture::validParams()
{
  InputParameters params = ACInterface::validParams();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel where crack propagation along weak"
                             "cleavage plane is preferred");
  params.addRequiredParam<Real>(
      "beta_penalty",
      "penalty to penalize fracture on planes not normal to one cleavage plane normal which is "
      "normal to weak cleavage plane. Setting beta=0 results in isotropic damage.");
  params.addRequiredParam<RealVectorValue>("cleavage_plane_normal",
                                           "Normal to the weak cleavage plane");
  return params;
}

ACInterfaceCleavageFracture::ACInterfaceCleavageFracture(const InputParameters & parameters)
  : ACInterface(parameters),
    _beta_penalty(getParam<Real>("beta_penalty")),
    _cleavage_plane_normal(getParam<RealVectorValue>("cleavage_plane_normal"))
{
}

Real
ACInterfaceCleavageFracture::betaNablaPsi()
{
  return _beta_penalty * _L[_qp] * _kappa[_qp] * (_grad_u[_qp] * _cleavage_plane_normal) *
         (_grad_test[_i][_qp] * _cleavage_plane_normal);
}

Real
ACInterfaceCleavageFracture::computeQpResidual()
{
  return (1 + _beta_penalty) * _grad_u[_qp] * kappaNablaLPsi() - betaNablaPsi();
}

Real
ACInterfaceCleavageFracture::computeQpJacobian()
{
  /// dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla
  /// (L\psi) \right) \f$
  RealGradient dsum =
      (_dkappadop[_qp] * _L[_qp] + _kappa[_qp] * _dLdop[_qp]) * _phi[_j][_qp] * _grad_test[_i][_qp];

  /// compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL =
        _grad_phi[_j][_qp] * _dLdop[_qp] + _grad_u[_qp] * _phi[_j][_qp] * _d2Ldop2[_qp];

    for (unsigned int i = 0; i < _n_args; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[i])[_qp];

    dsum += (_kappa[_qp] * dgradL + _dkappadop[_qp] * _phi[_j][_qp] * gradL()) * _test[_i][_qp];
  }

  return (1 + _beta_penalty) * _grad_phi[_j][_qp] * kappaNablaLPsi() + _grad_u[_qp] * dsum -
         _beta_penalty * _L[_qp] * _kappa[_qp] * (_grad_u[_qp] * _cleavage_plane_normal) *
             (_grad_phi[_j][_qp] * _cleavage_plane_normal);
}
