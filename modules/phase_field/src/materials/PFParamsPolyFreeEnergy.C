//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFParamsPolyFreeEnergy.h"

registerMooseObject("PhaseFieldApp", PFParamsPolyFreeEnergy);

InputParameters
PFParamsPolyFreeEnergy::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Phase field parameters for polynomial free energy for single component systems");
  params.addCoupledVar("T", "Temperature variable in Kelvin");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredParam<Real>(
      "int_width", "The interfacial width of void surface in the length scale of the problem");
  params.addParam<Real>(
      "length_scale", 1.0e-9, "defines the base length scale of the problem in m");
  params.addParam<Real>("time_scale", 1.0e-9, "defines the base time scale of the problem");
  MooseEnum poly_order("4 6 8");
  params.addRequiredParam<MooseEnum>(
      "polynomial_order", poly_order, "Order of polynomial free energy");
  params.addRequiredParam<Real>("D0", "Diffusivity prefactor for vacancies in m^2/s");
  params.addRequiredParam<Real>("Em", "Migration energy in eV");
  params.addRequiredParam<Real>("Ef", "Formation energy in eV");
  params.addRequiredParam<Real>("surface_energy", "Surface energy in J/m2");
  return params;
}

PFParamsPolyFreeEnergy::PFParamsPolyFreeEnergy(const InputParameters & parameters)
  : Material(parameters),
    _c(coupledValue("c")),
    _T(coupledValue("T")),
    _M(declareProperty<Real>("M")),
    _grad_M(declareProperty<RealGradient>("grad_M")),
    _kappa(declareProperty<Real>("kappa")),
    _c_eq(declareProperty<Real>("c_eq")),
    _W(declareProperty<Real>("barr_height")),
    _Qstar(declareProperty<Real>("Qstar")),
    _D(declareProperty<Real>("D")),
    _int_width(getParam<Real>("int_width")),
    _length_scale(getParam<Real>("length_scale")),
    _time_scale(getParam<Real>("time_scale")),
    _order(getParam<MooseEnum>("polynomial_order")),
    _D0(getParam<Real>("D0")),
    _Em(getParam<Real>("Em")),
    _Ef(getParam<Real>("Ef")),
    _surface_energy(getParam<Real>("surface_energy")),
    _JtoeV(6.24150974e18), // joule to eV conversion
    _kb(8.617343e-5)       // Boltzmann constant in eV/K
{
}

void
PFParamsPolyFreeEnergy::computeQpProperties()
{
  // Convert mobility from m^2/s to the length and time scale
  Real D0_c = _D0 * _time_scale / (_length_scale * _length_scale);

  Real kbT = _kb * _T[_qp];

  // Compute equilibrium concentration and diffusivity
  _c_eq[_qp] = std::exp(-_Ef / kbT);
  _D[_qp] = D0_c * std::exp(-_Em / kbT);

  // Compute free energy integral constant, such that int^1_0 f_loc = KN*sqrt(W)
  Real KN = 0.0;

  switch (_order)
  {
    case 0: // Fourth oder
      KN = 2.0 / 3.0;
      break;
    case 1: // Sixth order
      KN = 3.0 / 16.0 * std::sqrt(3.0) + 9.0 / 64.0 * std::sqrt(2.0) *
                                             (std::log(-std::sqrt(2.0) + std::sqrt(3.0)) +
                                              std::log(std::sqrt(2.0) + std::sqrt(3.0)));
      break;
    case 2: // Eigth order
      KN = 0.835510425;
      break;
    default:
      paramError("polynomial_order", "Incorrect polynomial order");
  }

  // Convert surface energy from J/m2 to eV/length_scale
  Real surf_energy = _surface_energy * _JtoeV * _length_scale * _length_scale;

  // Set interfacial parameter and energy barrier
  _kappa[_qp] = surf_energy * _int_width / KN;
  _W[_qp] = surf_energy / (2.0 * _int_width * KN);

  Real Co = 0.0;
  Real a = _c_eq[_qp];

  switch (_order)
  {
    case 0: // 4th order polynomial
      Co = std::pow(2.0, 5.0) * (1.0 + 2.0 * a - 2.0 * a * a);
      break;
    case 1: // 6th order polynomial
      Co = std::pow(2.0, 7.0) * (9.0 / 2.0 * a - 9.0 / 2.0 * a * a + 3.0 / 4.0);
      break;
    case 2: // 8th order polynomial
      Co = std::pow(2.0, 9.0) * (15.0 / 4.0 * a - 15.0 / 4.0 * a * a + 3.0 / 8.0);
      break;
    default:
      mooseError("Error in PFParamsPolyFreeEnergy: incorrect polynomial order");
  }

  _M[_qp] = KN / Co * (_D[_qp] * _int_width / surf_energy);
  _grad_M[_qp] = 0.0;

  _Qstar[_qp] = -4.0; // eV
}
