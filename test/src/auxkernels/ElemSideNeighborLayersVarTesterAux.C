//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemSideNeighborLayersVarTesterAux.h"
#include "ElemSideNeighborLayersVarTester.h"

registerMooseObject("MooseTestApp", ElemSideNeighborLayersVarTesterAux);

template <>
InputParameters
validParams<ElemSideNeighborLayersVarTesterAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>(
      "var_tester_uo",
      "The ElemSideNeighborLayersVarTester UserObject where this Aux pulls values from");

  params.addClassDescription(
      "Aux Kernel to display nodal values from a ElemSideNeighborLayersVarTester UserObject");
  return params;
}

ElemSideNeighborLayersVarTesterAux::ElemSideNeighborLayersVarTesterAux(
    const InputParameters & params)
  : AuxKernel(params), _uo(getUserObject<ElemSideNeighborLayersVarTester>("var_tester_uo"))
{
  if (!isNodal())
    mooseError("This AuxKernel only supports nodal fields");
}

Real
ElemSideNeighborLayersVarTesterAux::computeValue()
{
  return _uo.getNodalValue(_current_node->id());
}
