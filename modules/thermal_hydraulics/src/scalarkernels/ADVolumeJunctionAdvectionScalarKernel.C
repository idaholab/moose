//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVolumeJunctionAdvectionScalarKernel.h"
#include "ADVolumeJunctionBaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ADVolumeJunctionAdvectionKernel);
registerMooseObject("ThermalHydraulicsApp", ADVolumeJunctionAdvectionScalarKernel);

template <typename T>
InputParameters
ADVolumeJunctionAdvectionKernelTempl<T>::validParams()
{
  InputParameters params = T::validParams();

  params.addRequiredParam<unsigned int>("equation_index", "Equation index");
  params.addRequiredParam<UserObjectName>("volume_junction_uo", "Volume junction user object name");

  params.addClassDescription(
      "Adds advective fluxes for the junction variables for a volume junction");

  return params;
}

template <>
ADVolumeJunctionAdvectionKernelTempl<ADKernel>::ADVolumeJunctionAdvectionKernelTempl(
    const InputParameters & params)
  : ADKernel(params),
    _equation_index(getParam<unsigned int>("equation_index")),
    _volume_junction_uo(getUserObject<ADVolumeJunctionBaseUserObject>("volume_junction_uo"))
{
}

template <>
ADVolumeJunctionAdvectionKernelTempl<ADScalarKernel>::ADVolumeJunctionAdvectionKernelTempl(
    const InputParameters & params)
  : ADScalarKernel(params),
    _equation_index(getParam<unsigned int>("equation_index")),
    _volume_junction_uo(getUserObject<ADVolumeJunctionBaseUserObject>("volume_junction_uo"))
{
  if (_var.order() > 1)
    mooseError(name(), ": This scalar kernel can be used only with first-order scalar variables.");
}

template <typename T>
ADReal
ADVolumeJunctionAdvectionKernelTempl<T>::computeQpResidual()
{
  return _volume_junction_uo.getResidual()[_equation_index];
}
