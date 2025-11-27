//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearPNSFVMomentumPressure.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearPNSFVMomentumPressure);

InputParameters
LinearPNSFVMomentumPressure::validParams()
{
  InputParameters params = LinearFVMomentumPressure::validParams();
  params.addClassDescription("Porous momentum pressure gradient kernel that multiplies the "
                             "contribution by the local porosity.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity functor.");
  return params;
}

LinearPNSFVMomentumPressure::LinearPNSFVMomentumPressure(const InputParameters & params)
  : LinearFVMomentumPressure(params), _eps(getFunctor<Real>(NS::porosity))
{
}

Real
LinearPNSFVMomentumPressure::computeRightHandSideContribution()
{
  const auto dof_value = _current_elem_info->dofIndices()[_pressure_sys_num][_pressure_var_num];
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  const Real porosity = _eps(elem_arg, state);
  return -porosity * (*_pressure_gradient[_index])(dof_value) * _current_elem_volume;
}
