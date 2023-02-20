//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSMachAux.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", NSMachAux);

InputParameters
NSMachAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription(
      "Auxiliary kernel for computing the Mach number assuming an ideal gas.");
  params.addParam<bool>("use_material_properties",
                        false,
                        "Whether to use material properties to compute the Mach number");
  params.addCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // Only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // Only required in 3D...
  params.addCoupledVar(NS::specific_volume, "specific volume");
  params.addCoupledVar(NS::specific_internal_energy, "internal energy");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSMachAux::NSMachAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _use_mat_props(getParam<bool>("use_material_properties")),
    _u_vel(_use_mat_props ? nullptr : &coupledValue(NS::velocity_x)),
    _v_vel(_use_mat_props ? nullptr
                          : (_mesh.dimension() >= 2 ? &coupledValue(NS::velocity_y) : &_zero)),
    _w_vel(_use_mat_props ? nullptr
                          : (_mesh.dimension() == 3 ? &coupledValue(NS::velocity_z) : &_zero)),
    _specific_volume(_use_mat_props ? nullptr : &coupledValue(NS::specific_volume)),
    _specific_internal_energy(_use_mat_props ? nullptr
                                             : &coupledValue(NS::specific_internal_energy)),
    _mat_speed(_use_mat_props ? &getADMaterialProperty<Real>(NS::speed) : nullptr),
    _mat_pressure(_use_mat_props ? &getADMaterialProperty<Real>(NS::pressure) : nullptr),
    _mat_T_fluid(_use_mat_props ? &getADMaterialProperty<Real>(NS::T_fluid) : nullptr),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

Real
NSMachAux::computeValue()
{
  if (_use_mat_props)
    return MetaPhysicL::raw_value((*_mat_speed)[_qp] /
                                  _fp.c_from_p_T((*_mat_pressure)[_qp], (*_mat_T_fluid)[_qp]));
  else
    return RealVectorValue((*_u_vel)[_qp], (*_v_vel)[_qp], (*_w_vel)[_qp]).norm() /
           _fp.c_from_v_e((*_specific_volume)[_qp], (*_specific_internal_energy)[_qp]);
}
