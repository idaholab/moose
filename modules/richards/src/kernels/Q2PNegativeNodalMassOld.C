/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Q2PNegativeNodalMassOld.h"

#include <iostream>

template <>
InputParameters
validParams<Q2PNegativeNodalMassOld>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addRequiredParam<UserObjectName>(
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredCoupledVar("other_var",
                               "The other variable in the 2-phase system.  If "
                               "Variable=porepressure, then other_var should be the "
                               "saturation Variable, and vice-versa.");
  params.addRequiredParam<bool>(
      "var_is_porepressure",
      "This flag is needed to correctly calculate the Jacobian entries.  If "
      "set to true, this Kernel will assume it is describing the mass of "
      "the phase with porepressure as its Variable (eg, the liquid phase).  "
      "If set to false, this Kernel will assumed it is describing the mass "
      "of the phase with saturation as its variable (eg, the gas phase)");
  params.addClassDescription("- fluid_mass");
  return params;
}

Q2PNegativeNodalMassOld::Q2PNegativeNodalMassOld(const InputParameters & parameters)
  : TimeKernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _other_var_nodal_old(coupledNodalValueOld("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure")),
    _porosity_old(getMaterialProperty<Real>("porosity_old"))
{
}

Real
Q2PNegativeNodalMassOld::computeQpResidual()
{
  Real density_old;
  Real mass_old;

  if (_var_is_pp)
  {
    density_old = _density.density(_var.nodalSlnOld()[_i]);
    mass_old = _porosity_old[_qp] * density_old * (1 - _other_var_nodal_old[_i]);
  }
  else
  {
    density_old = _density.density(_other_var_nodal_old[_i]);
    mass_old = _porosity_old[_qp] * density_old * _var.nodalSlnOld()[_i];
  }

  return _test[_i][_qp] * (-mass_old) / _dt;
}
