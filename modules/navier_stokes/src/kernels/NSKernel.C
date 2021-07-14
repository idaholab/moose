//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NS.h"
#include "NSKernel.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

namespace nms = NS;

InputParameters
NSKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("This class couples together all the variables for the compressible "
                             "Navier-Stokes equations to allow them to be used in derived Kernel "
                             "classes.");
  params.addRequiredCoupledVar(nms::velocity_x, "x-velocity");
  params.addCoupledVar(nms::velocity_y, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar(nms::velocity_z, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar(nms::density, "density");
  params.addRequiredCoupledVar(nms::momentum_x, "x-momentum");
  params.addCoupledVar(nms::momentum_y, "y-momentum"); // only required in 2D and 3D
  params.addCoupledVar(nms::momentum_z, "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar(nms::total_energy_density, "total energy density");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NSKernel::NSKernel(const InputParameters & parameters)
  : Kernel(parameters),
    // Coupled variables
    _u_vel(coupledValue(nms::velocity_x)),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue(nms::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(nms::velocity_z) : _zero),

    _rho(coupledValue(nms::density)),
    _rho_u(coupledValue(nms::momentum_x)),
    _rho_v(_mesh.dimension() >= 2 ? coupledValue(nms::momentum_y) : _zero),
    _rho_w(_mesh.dimension() == 3 ? coupledValue(nms::momentum_z) : _zero),
    _rho_et(coupledValue(nms::total_energy_density)),

    // Gradients
    _grad_rho(coupledGradient(nms::density)),
    _grad_rho_u(coupledGradient(nms::momentum_x)),
    _grad_rho_v(_mesh.dimension() >= 2 ? coupledGradient(nms::momentum_y) : _grad_zero),
    _grad_rho_w(_mesh.dimension() == 3 ? coupledGradient(nms::momentum_z) : _grad_zero),
    _grad_rho_et(coupledGradient(nms::total_energy_density)),

    // Variable numberings
    _rho_var_number(coupled(nms::density)),
    _rhou_var_number(coupled(nms::momentum_x)),
    _rhov_var_number(_mesh.dimension() >= 2 ? coupled(nms::momentum_y) : libMesh::invalid_uint),
    _rhow_var_number(_mesh.dimension() == 3 ? coupled(nms::momentum_z) : libMesh::invalid_uint),
    _rho_et_var_number(coupled(nms::total_energy_density)),

    // Material properties
    _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
    _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),

    // FluidProperties UserObject
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

bool
NSKernel::isNSVariable(unsigned var)
{
  if (var == _rho_var_number || var == _rhou_var_number || var == _rhov_var_number ||
      var == _rhow_var_number || var == _rho_et_var_number)
    return true;
  else
    return false;
}

unsigned
NSKernel::mapVarNumber(unsigned var)
{
  // Convert the Moose numbering to:
  // 0 for rho
  // 1 for rho*u
  // 2 for rho*v
  // 3 for rho*w
  // 4 for rho*e
  // regardless of the problem dimension, etc.
  unsigned mapped_var_number = 99;

  if (var == _rho_var_number)
    mapped_var_number = 0;
  else if (var == _rhou_var_number)
    mapped_var_number = 1;
  else if (var == _rhov_var_number)
    mapped_var_number = 2;
  else if (var == _rhow_var_number)
    mapped_var_number = 3;
  else if (var == _rho_et_var_number)
    mapped_var_number = 4;
  else
    mooseError("Invalid var!");

  return mapped_var_number;
}
