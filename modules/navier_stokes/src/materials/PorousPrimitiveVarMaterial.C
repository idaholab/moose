//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "PorousPrimitiveVarMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", PorousPrimitiveVarMaterial);

InputParameters
PorousPrimitiveVarMaterial::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addRequiredCoupledVar(NS::T_fluid, "The fluid temperature");
  params.addRequiredCoupledVar(NS::superficial_velocity_x, "The x-velocity times the porosity");
  params.addCoupledVar(NS::superficial_velocity_y, "The y-velocity times the porosity");
  params.addCoupledVar(NS::superficial_velocity_z, "The z-velocity times the porosity");
  params.addClassDescription("Provides access to variables for a primitive variable set "
                             "of pressure, temperature, and superficial velocity");
  params.addRequiredParam<MaterialPropertyName>(NS::porosity, "the porosity");
  return params;
}

PorousPrimitiveVarMaterial::PorousPrimitiveVarMaterial(const InputParameters & params)
  : Material(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _var_pressure(adCoupledValue(NS::pressure)),
    _var_ud(adCoupledValue(NS::superficial_velocity_x)),
    _var_vd(isCoupled(NS::superficial_velocity_y) ? adCoupledValue(NS::superficial_velocity_y)
                                                  : _ad_zero),
    _var_wd(isCoupled(NS::superficial_velocity_z) ? adCoupledValue(NS::superficial_velocity_z)
                                                  : _ad_zero),
    _var_T_fluid(adCoupledValue(NS::T_fluid)),
    _epsilon(getMaterialProperty<Real>(NS::porosity)),
    _rho(declareADProperty<Real>(NS::density)),
    _momentum(declareADProperty<RealVectorValue>(NS::momentum)),
    _superficial_velocity(declareADProperty<RealVectorValue>(NS::superficial_velocity)),
    _vel_x(declareADProperty<Real>(NS::velocity_x)),
    _vel_y(declareADProperty<Real>(NS::velocity_y)),
    _vel_z(declareADProperty<Real>(NS::velocity_z)),
    _mom_x(declareADProperty<Real>(NS::momentum_x)),
    _mom_y(declareADProperty<Real>(NS::momentum_y)),
    _mom_z(declareADProperty<Real>(NS::momentum_z)),
    _specific_internal_energy(declareADProperty<Real>(NS::specific_internal_energy)),
    _pressure(declareADProperty<Real>(NS::pressure)),
    _rho_ht(declareADProperty<Real>(NS::total_enthalpy_density)),
    _T_fluid(declareADProperty<Real>(NS::T_fluid))
{
}

void
PorousPrimitiveVarMaterial::computeQpProperties()
{
  // Our primitive variable set
  _pressure[_qp] = _var_pressure[_qp];
  _T_fluid[_qp] = _var_T_fluid[_qp];
  _superficial_velocity[_qp] = {_var_ud[_qp], _var_vd[_qp], _var_wd[_qp]};

  // Our fourth independent variable (computed using our equation of state)
  _rho[_qp] = _fluid.rho_from_p_T(_pressure[_qp], _T_fluid[_qp]);

  // Perhaps only useful for visualization (e.g. ADMaterialRealAux)
  const auto velocity = _superficial_velocity[_qp] / _epsilon[_qp];
  _vel_x[_qp] = velocity(0);
  _vel_y[_qp] = velocity(1);
  _vel_z[_qp] = velocity(2);

  // Potentially consumed by objects like PCNSFVLaxFriedrichs
  _momentum[_qp] = velocity * _rho[_qp];
  _mom_x[_qp] = _momentum[_qp](0);
  _mom_y[_qp] = _momentum[_qp](1);
  _mom_z[_qp] = _momentum[_qp](2);
  _specific_internal_energy[_qp] = _fluid.e_from_p_rho(_pressure[_qp], _rho[_qp]);
  const auto et = _specific_internal_energy[_qp] + 0.5 * velocity * velocity;
  _rho_ht[_qp] = _rho[_qp] * et + _pressure[_qp];
}
