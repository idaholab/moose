/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMMaterialManagerDiracKernel.h"

#include "XFEMMaterialManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<XFEMMaterialManagerDiracKernel>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<UserObjectName>("manager", "XFEMMaterialManager object");
  return params;
}

XFEMMaterialManagerDiracKernel::XFEMMaterialManagerDiracKernel(const InputParameters & parameters)
  : DiracKernel(parameters), _manager(getUserObject<XFEMMaterialManager>("manager"))
{
}

void
XFEMMaterialManagerDiracKernel::addPoints()
{
  const auto & map = _manager.getExtraQPMap();
  for (auto element : map)
  {
    const Elem * elem = _mesh.getMesh().elem_ptr(element.first);
    for (auto point : element.second)
      addPoint(elem, point);
  }
}

void
XFEMMaterialManagerDiracKernel::computeResidual()
{
  _manager.swapInProperties(_current_elem->id());
  DiracKernel::computeResidual();
  _manager.swapOutProperties(_current_elem->id());
}

void
XFEMMaterialManagerDiracKernel::computeJacobian()
{
  _manager.swapInProperties(_current_elem->id());
  DiracKernel::computeJacobian();
  _manager.swapOutProperties(_current_elem->id());
}
