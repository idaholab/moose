/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ManagedSourceTest.h"

#include "XFEMMaterialManager.h"

template <>
InputParameters
validParams<ManagedSourceTest>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addClassDescription("Tests the XFEMMaterialManager");
  params.addRequiredParam<UserObjectName>("manager", "XFEMMaterialManager object");
  return params;
}

ManagedSourceTest::ManagedSourceTest(const InputParameters & parameters)
  : DiracKernel(parameters), _manager(getUserObject<XFEMMaterialManager>("manager"))
{
}

void
ManagedSourceTest::addPoints()
{
}

Real
ManagedSourceTest::computeQpResidual()
{
  return 0.0;
}

Real
ManagedSourceTest::computeQpJacobian()
{
  return 0.0;
}
