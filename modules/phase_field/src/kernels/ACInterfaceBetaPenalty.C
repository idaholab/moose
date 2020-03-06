// Considers cleavage plane anisotropy in the crack propagation

#include "ACInterfaceBetaPenalty.h"

registerMooseObject("PhaseFieldApp", ACInterfaceBetaPenalty);

template <>
InputParameters
validParams<ACInterfaceBetaPenalty>()
{
  InputParameters params = validParams<ACInterface>();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel where crack propagation along weak "
                             "cleavage plane is preferred");
  params.addRequiredParam<Real>(
      "beta_penalty",
      "penalty to penalize fracture on planes not normal to one cleavage plane "
      "normal which is normal to weak cleavage plane. Setting beta=0 results "
      "in isotropic damage.");
  params.addRequiredParam<std::vector<Real>>("cleavage_plane_normal",
                                             "Normal to the weak cleavage plane");
  return params;
}

ACInterfaceBetaPenalty::ACInterfaceBetaPenalty(const InputParameters & parameters)
  : ACInterface(parameters),
    _beta_penalty(getParam<Real>("beta_penalty")),
    _cleavage_plane_normal(getParam<std::vector<Real>>("cleavage_plane_normal"))
{
}

Real
ACInterfaceBetaPenalty::betaNablaPsi()
{
  Real ngradc;
  ngradc = _grad_u[_qp](0) * _cleavage_plane_normal[0] +
           _grad_u[_qp](1) * _cleavage_plane_normal[1] +
           _grad_u[_qp](2) * _cleavage_plane_normal[2];

  Real ngradpsi;
  ngradpsi = _grad_test[_i][_qp](0) * _cleavage_plane_normal[0] +
             _grad_test[_i][_qp](1) * _cleavage_plane_normal[1] +
             _grad_test[_i][_qp](2) * _cleavage_plane_normal[2];

  return _beta_penalty * _L[_qp] * _kappa[_qp] * ngradc * ngradpsi;
}

Real
ACInterfaceBetaPenalty::computeQpResidual()
{
  return (1 + _beta_penalty) * _grad_u[_qp] * kappaNablaLPsi() - betaNablaPsi();
}

Real
ACInterfaceBetaPenalty::computeQpJacobian()
{
  // dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla
  // (L\psi) \right) \f$
  RealGradient dsum =
      (_dkappadop[_qp] * _L[_qp] + _kappa[_qp] * _dLdop[_qp]) * _phi[_j][_qp] * _grad_test[_i][_qp];

  // compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL =
        _grad_phi[_j][_qp] * _dLdop[_qp] + _grad_u[_qp] * _phi[_j][_qp] * _d2Ldop2[_qp];

    for (unsigned int i = 0; i < _nvar; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[i])[_qp];

    dsum += (_kappa[_qp] * dgradL + _dkappadop[_qp] * _phi[_j][_qp] * gradL()) * _test[_i][_qp];
  }

  Real ngradc;
  ngradc = _grad_u[_qp](0) * _cleavage_plane_normal[0] +
           _grad_u[_qp](1) * _cleavage_plane_normal[1] +
           _grad_u[_qp](2) * _cleavage_plane_normal[2];

  Real ngradphi;
  ngradphi = _grad_phi[_j][_qp](0) * _cleavage_plane_normal[0] +
             _grad_phi[_j][_qp](1) * _cleavage_plane_normal[1] +
             _grad_phi[_j][_qp](2) * _cleavage_plane_normal[2];

  return (1 + _beta_penalty) * _grad_phi[_j][_qp] * kappaNablaLPsi() + _grad_u[_qp] * dsum -
         _beta_penalty * _L[_qp] * _kappa[_qp] * ngradc * ngradphi;
}
