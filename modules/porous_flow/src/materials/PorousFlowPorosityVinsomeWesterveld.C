//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityVinsomeWesterveld.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <cmath>

registerMooseObject("PorousFlowApp", PorousFlowPorosityVinsomeWesterveld);

InputParameters
PorousFlowPorosityVinsomeWesterveld::validParams()
{
  InputParameters params = PorousFlowPorosityLinear::validParams();
  params.addRequiredParam<Real>(
      "thermal_opening_coeff",
      "Change in porosity per unit heat content of the adjacent matrix.  The heat content, "
      "I = int (T - matrix_reference_temperature) dz, has units K.m, so when porosity represents "
      "the aperture of a lower-dimensional fracture this coefficient has units 1/K.  Porosity "
      "changes by -thermal_opening_coeff * I, so cooling (I < 0) opens the fracture when this "
      "coefficient is positive");
  params.addRequiredRangeCheckedParam<Real>("matrix_thermal_diffusivity",
                                            "matrix_thermal_diffusivity > 0",
                                            "Thermal diffusivity of the matrix adjacent to the "
                                            "fracture (thermal conductivity divided by "
                                            "volumetric heat capacity), m^2/s");
  params.addRequiredCoupledVar(
      "matrix_reference_temperature",
      "Undisturbed temperature of the matrix adjacent to the fracture.  This should usually be a "
      "linear-lagrange variable or a constant");
  params.addParam<Real>(
      "vw_start_time",
      0.0,
      "Time at which the temperature at the fracture face first starts to change "
      "(for instance, when injection begins).  The matrix adjacent to the fracture "
      "is at matrix_reference_temperature at this time, and the thermal-opening "
      "term is zero before it");
  params.addClassDescription(
      "This Material calculates porosity (usually the aperture of a lower-dimensional fracture) "
      "using the PorousFlowPorosityLinear relationship plus a thermal-opening term proportional to "
      "the heat content of the adjacent matrix, which is computed using the semi-analytic method "
      "of Vinsome and Westerveld (1980)");
  return params;
}

PorousFlowPorosityVinsomeWesterveld::PorousFlowPorosityVinsomeWesterveld(
    const InputParameters & parameters)
  : PorousFlowPorosityLinear(parameters),
    _thermal_opening_coeff(getParam<Real>("thermal_opening_coeff")),
    _kappa(getParam<Real>("matrix_thermal_diffusivity")),
    _T_matrix_ref(nodalOrQpValue("matrix_reference_temperature")),
    _start_time(getParam<Real>("vw_start_time")),
    _theta(_nodal_material ? declareProperty<Real>("PorousFlow_vw_theta_nodal")
                           : declareProperty<Real>("PorousFlow_vw_theta_qp")),
    _theta_old(_nodal_material ? getMaterialPropertyOld<Real>("PorousFlow_vw_theta_nodal")
                               : getMaterialPropertyOld<Real>("PorousFlow_vw_theta_qp")),
    _heat_content(_nodal_material ? declareProperty<Real>("PorousFlow_vw_heat_content_nodal")
                                  : declareProperty<Real>("PorousFlow_vw_heat_content_qp")),
    _heat_content_old(_nodal_material
                          ? getMaterialPropertyOld<Real>("PorousFlow_vw_heat_content_nodal")
                          : getMaterialPropertyOld<Real>("PorousFlow_vw_heat_content_qp"))
{
}

void
PorousFlowPorosityVinsomeWesterveld::initialSetup()
{
  PorousFlowPorosityLinear::initialSetup();
  if (!(_temperature && _dtemperature_dvar))
    mooseError("PorousFlowPorosityVinsomeWesterveld: a temperature Material is required");
}

void
PorousFlowPorosityVinsomeWesterveld::initQpStatefulProperties()
{
  PorousFlowPorosityLinear::initQpStatefulProperties();
  // The matrix starts at its reference temperature
  _theta[_qp] = 0.0;
  _heat_content[_qp] = 0.0;
}

void
PorousFlowPorosityVinsomeWesterveld::computeUnboundedPorosityQp()
{
  PorousFlowPorosityLinear::computeUnboundedPorosityQp();

  const Real theta = (*_temperature)[_qp] - _T_matrix_ref[_qp];
  _theta[_qp] = theta;

  // dI/dtheta, used for the Jacobian
  Real dI_dtheta = 0.0;

  const Real time = _t - _start_time;
  const Real a = _kappa * std::min(_dt, time);
  if (time <= 0.0)
    _heat_content[_qp] = 0.0;
  else if (a <= 0.0)
    _heat_content[_qp] = _heat_content_old[_qp];
  else
  {
    // Vinsome and Westerveld (1980): the profile is
    // (theta + p z + q z^2) exp(-z / d), with d = sqrt(kappa t) / 2.
    // Imposing the heat-conduction equation at z = 0 and the energy balance
    // dI/dt = kappa * (theta / d - p) over the time step gives p and I below.
    const Real d = 0.5 * std::sqrt(_kappa * time);
    const Real denom = 3.0 * d * d + a;
    const Real p = (a * theta / d + _heat_content_old[_qp] -
                    Utility::pow<3>(d) * (theta - _theta_old[_qp]) / a) /
                   denom;
    _heat_content[_qp] = _heat_content_old[_qp] + a * (theta / d - p);

    const Real dp_dtheta = (a / d - Utility::pow<3>(d) / a) / denom;
    dI_dtheta = a * (1.0 / d - dp_dtheta);
  }

  _porosity[_qp] -= _thermal_opening_coeff * _heat_content[_qp];
  for (unsigned int pvar = 0; pvar < _num_var; ++pvar)
    (*_dporosity_dvar)[_qp][pvar] -=
        _thermal_opening_coeff * dI_dtheta * (*_dtemperature_dvar)[_qp][pvar];
}
