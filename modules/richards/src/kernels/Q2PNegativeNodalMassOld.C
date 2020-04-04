//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Q2PNegativeNodalMassOld.h"

// MOOSE includes
#include "MooseVariable.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", Q2PNegativeNodalMassOld);

InputParameters
Q2PNegativeNodalMassOld::validParams()
{
  InputParameters params = TimeKernel::validParams();
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
    _other_var_nodal_old(coupledDofValuesOld("other_var")),
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
    density_old = _density.density(_var.dofValuesOld()[_i]);
    mass_old = _porosity_old[_qp] * density_old * (1 - _other_var_nodal_old[_i]);
  }
  else
  {
    density_old = _density.density(_other_var_nodal_old[_i]);
    mass_old = _porosity_old[_qp] * density_old * _var.dofValuesOld()[_i];
  }

  return _test[_i][_qp] * (-mass_old) / _dt;
}
