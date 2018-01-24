/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InterfaceDiffusionBase.h"

template <>
InputParameters
validParams<InterfaceDiffusionBase>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addParam<Real>("D", 1.0, "Diffusion coefficient");
  params.addParam<Real>("D_neighbor", 1.0, "Neighbor variable diffusion coefficient");
  return params;
}

InterfaceDiffusionBase::InterfaceDiffusionBase(const InputParameters & parameters)
  : InterfaceKernel(parameters), _D(getParam<Real>("D")), _D_neighbor(getParam<Real>("D_neighbor"))
{
}
