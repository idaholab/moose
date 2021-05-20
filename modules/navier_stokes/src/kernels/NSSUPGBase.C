//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSSUPGBase.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

InputParameters
NSSUPGBase::validParams()
{
  InputParameters params = NSKernel::validParams();
  params.addClassDescription("This class acts as a base class for stabilization kernels.");
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  params.addRequiredCoupledVar(NS::specific_total_enthalpy, "specific total enthalpy");
  return params;
}

NSSUPGBase::NSSUPGBase(const InputParameters & parameters)
  : NSKernel(parameters),

    // Material properties
    _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
    _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),

    // SUPG-related material properties
    _hsupg(getMaterialProperty<Real>("hsupg")),
    _tauc(getMaterialProperty<Real>("tauc")),
    _taum(getMaterialProperty<Real>("taum")),
    _taue(getMaterialProperty<Real>("taue")),
    _strong_residuals(getMaterialProperty<std::vector<Real>>("strong_residuals")),

    // Momentum equation inviscid flux matrices
    _calA(getMaterialProperty<std::vector<RealTensorValue>>("calA")),

    // "velocity column" matrices
    _calC(getMaterialProperty<std::vector<RealTensorValue>>("calC")),

    // energy inviscid flux matrices
    _calE(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>("calE")),

    // Old coupled variable values
    // _rho_old(coupledValueOld(NS::density)),
    // _rho_u_old(coupledValueOld(NS::momentum_x)),
    // _rho_v_old(_mesh.dimension() >= 2 ? coupledValueOld(NS::momentum_y) : _zero),
    // _rho_w_old(_mesh.dimension() == 3 ? coupledValueOld(NS::momentum_z) : _zero),
    // _rho_et_old(coupledValueOld(NS::total_energy_density)),

    // Time derivative derivatives (no, that's not a typo).  You can
    // just think of these as 1/dt for simplicity, they usually are...
    _d_rhodot_du(coupledDotDu(NS::density)),
    _d_rhoudot_du(coupledDotDu(NS::momentum_x)),
    _d_rhovdot_du(_mesh.dimension() >= 2 ? coupledDotDu(NS::momentum_y) : _zero),
    _d_rhowdot_du(_mesh.dimension() == 3 ? coupledDotDu(NS::momentum_z) : _zero),
    _d_rho_etdot_du(coupledDotDu(NS::total_energy_density)),

    // Coupled aux variables
    _temperature(coupledValue(NS::temperature)),
    _specific_total_enthalpy(coupledValue(NS::specific_total_enthalpy))
{
}
