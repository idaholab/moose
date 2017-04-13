/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSSUPGBase.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSSUPGBase>()
{
  InputParameters params = validParams<NSKernel>();
  params.addClassDescription("This class acts as a base class for stabilization kernels.");
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  params.addRequiredCoupledVar(NS::enthalpy, "total enthalpy");
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
    // _rho_E_old(coupledValueOld(NS::total_energy)),

    // Time derivative derivatives (no, that's not a typo).  You can
    // just think of these as 1/dt for simplicity, they usually are...
    _d_rhodot_du(coupledDotDu(NS::density)),
    _d_rhoudot_du(coupledDotDu(NS::momentum_x)),
    _d_rhovdot_du(_mesh.dimension() >= 2 ? coupledDotDu(NS::momentum_y) : _zero),
    _d_rhowdot_du(_mesh.dimension() == 3 ? coupledDotDu(NS::momentum_z) : _zero),
    _d_rhoEdot_du(coupledDotDu(NS::total_energy)),

    // Coupled aux variables
    _temperature(coupledValue(NS::temperature)),
    _enthalpy(coupledValue("enthalpy"))
{
}
