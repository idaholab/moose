/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVMaterial.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<CNSFVMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addClassDescription("A material kernel for the CNS equations.");

  params.addRequiredCoupledVar("rho", "Conserved variable: rho");

  params.addRequiredCoupledVar("rhou", "Conserved variable: rhou");

  params.addCoupledVar("rhov", "Conserved variable: rhov");

  params.addCoupledVar("rhow", "Conserved variable: rhow");

  params.addRequiredCoupledVar("rhoe", "Conserved variable: rhoe");

  params.addRequiredParam<UserObjectName>("slope_limiting", "Name for slope limiting user object");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVMaterial::CNSFVMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rhoc(coupledValue("rho")),
    _rhouc(coupledValue("rhou")),
    _rhovc(isCoupled("rhov") ? coupledValue("rhov") : _zero),
    _rhowc(isCoupled("rhow") ? coupledValue("rhow") : _zero),
    _rhoec(coupledValue("rhoe")),
    _lslope(getUserObject<SlopeLimitingBase>("slope_limiting")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _rho(declareProperty<Real>("rho")),
    _rhou(declareProperty<Real>("rhou")),
    _rhov(declareProperty<Real>("rhov")),
    _rhow(declareProperty<Real>("rhow")),
    _rhoe(declareProperty<Real>("rhoe")),
    _vmag(declareProperty<Real>("velocity_magnitude")),
    _pres(declareProperty<Real>("pressure")),
    _temp(declareProperty<Real>("temperature")),
    _enth(declareProperty<Real>("enthalpy")),
    _csou(declareProperty<Real>("speed_of_sound")),
    _mach(declareProperty<Real>("mach_number")),
    _uadv(declareProperty<Real>("vel_x")),
    _vadv(declareProperty<Real>("vel_y")),
    _wadv(declareProperty<Real>("vel_z"))
{
}

CNSFVMaterial::~CNSFVMaterial() {}

void
CNSFVMaterial::computeQpProperties()
{
  /// initialize the conserved variables: rho, rhou, rhov, rhow, rhoe
  _rho[_qp] = _rhoc[_qp];
  _rhou[_qp] = _rhouc[_qp];
  _rhov[_qp] = _rhovc[_qp];
  _rhow[_qp] = _rhowc[_qp];
  _rhoe[_qp] = _rhoec[_qp];

  /// calculate the primitive variables: pres, uadv, vadv, wadv, temp
  _uadv[_qp] = _rhou[_qp] / _rho[_qp];
  _vadv[_qp] = _rhov[_qp] / _rho[_qp];
  _wadv[_qp] = _rhow[_qp] / _rho[_qp];

  Real vdov = _uadv[_qp] * _uadv[_qp] + _vadv[_qp] * _vadv[_qp] + _wadv[_qp] * _wadv[_qp];

  Real v = 1. / _rho[_qp];
  Real e = _rhoe[_qp] / _rho[_qp] - 0.5 * vdov;

  _pres[_qp] = _fp.pressure(v, e);
  _temp[_qp] = _fp.temperature(v, e);

  /// interpolate variable values at face center
  if (_bnd)
  {
    /// reconstruct the slopes of primitive variables

    unsigned int nvars = 5;
    std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));
    ugrad = _lslope.getElementSlope(_current_elem->id());

    /// get the directional vector from cell center to face center

    RealGradient dvec = _q_point[_qp] - _current_elem->centroid();

    /// calculate the conserved variables at face center

    _pres[_qp] += ugrad[0] * dvec;
    _uadv[_qp] += ugrad[1] * dvec;
    _vadv[_qp] += ugrad[2] * dvec;
    _wadv[_qp] += ugrad[3] * dvec;
    _temp[_qp] += ugrad[4] * dvec;

    _rho[_qp] = _fp.rho(_pres[_qp], _temp[_qp]);

    _rhou[_qp] = _rho[_qp] * _uadv[_qp];

    _rhov[_qp] = _rho[_qp] * _vadv[_qp];

    _rhow[_qp] = _rho[_qp] * _wadv[_qp];

    _rhoe[_qp] = _rho[_qp] * _fp.e(_pres[_qp], _rho[_qp]) +
                 _rho[_qp] * 0.5 *
                     (_uadv[_qp] * _uadv[_qp] + _vadv[_qp] * _vadv[_qp] + _wadv[_qp] * _wadv[_qp]);

    /// clear the temporary vectors

    ugrad.clear();
  }

  /// calculations only for elemental output
  else if (!_bnd)
  {
    _vmag[_qp] = std::sqrt(vdov);

    _csou[_qp] = _fp.c(v, e);

    _enth[_qp] = (_rhoe[_qp] + _pres[_qp]) / _rho[_qp];

    _mach[_qp] = std::sqrt(vdov) / _csou[_qp];
  }
}
