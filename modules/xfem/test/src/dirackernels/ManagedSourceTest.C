/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ManagedSourceTest.h"

#include "XFEMMaterialManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ManagedSourceTest>()
{
  InputParameters params = validParams<XFEMMaterialManagerDiracKernel>();
  params.addClassDescription("Tests the XFEMMaterialManager");
  return params;
}

ManagedSourceTest::ManagedSourceTest(const InputParameters & parameters)
  : XFEMMaterialManagerDiracKernel(parameters)
{
}

void
ManagedSourceTest::initialSetup()
{
  _prop1 = getMaterialProperty<Real>("prop1");
  _prop3 = getMaterialProperty<Real>("prop3");
}

Real
ManagedSourceTest::computeQpResidual()
{
  std::cout << "elem " << _current_elem << " point " << _qp << ':' << _q_point[_qp]
            << " prop1=" << (*_prop1)[_qp] << " prop3=" << (*_prop3)[_qp] << '\n';
  return 0.0;
}

Real
ManagedSourceTest::computeQpJacobian()
{
  return 0.0;
}
