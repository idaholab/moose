//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KineticEnergyAux.h"

registerMooseObject("TensorMechanicsApp", KineticEnergyAux);
registerMooseObject("TensorMechanicsApp", ADKineticEnergyAux);

template <bool is_ad>
InputParameters
KineticEnergyAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute the kinetic energy of continuum-based finite elements");
  params.addRequiredCoupledVar("newmark_velocity_x",
                               "X component of the velocity from the Newmark integration scheme");
  params.addRequiredCoupledVar("newmark_velocity_y",
                               "Y component of the velocity from the Newmark integration scheme");
  params.addRequiredCoupledVar("newmark_velocity_z",
                               "Z component of the velocity from the Newmark integration scheme");

  params.addParam<MaterialPropertyName>("density",
                                        "density",
                                        "Name of material property or a constant real number "
                                        "defining the density of the continuum material.");
  params.addParam<std::string>("base_name", "Mechanical property base name");
  return params;
}

template <bool is_ad>
KineticEnergyAuxTempl<is_ad>::KineticEnergyAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _density(getGenericMaterialProperty<Real, is_ad>(_base_name + "density")),
    _vel_x(coupledValue("newmark_velocity_x")),
    _vel_y(coupledValue("newmark_velocity_y")),
    _vel_z(coupledValue("newmark_velocity_z"))
{
}

template <bool is_ad>
Real
KineticEnergyAuxTempl<is_ad>::computeValue()
{
  const Real kinetic_energy = MetaPhysicL::raw_value(
      0.5 * _density[_qp] *
      (_vel_x[_qp] * _vel_x[_qp] + _vel_y[_qp] * _vel_y[_qp] + _vel_z[_qp] * _vel_z[_qp]));
  return kinetic_energy;
}

template class KineticEnergyAuxTempl<false>;
template class KineticEnergyAuxTempl<true>;
