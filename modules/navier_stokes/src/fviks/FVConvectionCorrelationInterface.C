//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVConvectionCorrelationInterface.h"
#include "MooseVariableFV.h"

registerMooseObject("NavierStokesApp", FVConvectionCorrelationInterface);

InputParameters
FVConvectionCorrelationInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription("Computes the residual for a convective heat transfer across an "
                             "interface for the finite volume method, "
                             "using a correlation for the heat transfer coefficient.");
  params.addRequiredCoupledVar("temp_fluid", "The fluid temperature variable");
  params.addRequiredCoupledVar("temp_solid", "The solid/wall temperature variable");
  params.addRequiredParam<MaterialPropertyName>("h", "The convective heat transfer coefficient");
  params.addParam<Real>(
      "bulk_distance", -1, "The distance to the bulk for evaluating the fluid bulk temperature");
  params.addParam<bool>("wall_cell_is_bulk",
                        false,
                        "Use the wall cell centroid temperature for the fluid bulk temperature");
  return params;
}

FVConvectionCorrelationInterface::FVConvectionCorrelationInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _temp_fluid(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("temp_fluid", 0))),
    _temp_solid(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("temp_solid", 0))),
    _htc_elem(getADMaterialProperty<Real>("h")),
    _htc_neighbor(getNeighborADMaterialProperty<Real>("h")),
    _bulk_distance(getParam<Real>("bulk_distance")),
    _use_wall_cell(getParam<bool>("wall_cell_is_bulk")),
    _pl(mesh().getPointLocator())
{
  if (!_use_wall_cell && (_bulk_distance < 0))
    mooseError(
        "The bulk distance should be specified or 'wall_cell_is_bulk' should be set to true for "
        "the FVTwoVarConvectionCorrelationInterface");
  if (&var1() != _temp_fluid)
    paramError("temp_fluid", "variable1 must be equal to temp_fluid.");
  if (&var2() != _temp_solid)
    paramError("temp_solid", "variable2 must be equal to temp_solid.");
}

ADReal
FVConvectionCorrelationInterface::computeQpResidual()
{
  const Elem * current_elem = elemIsOne() ? &_face_info->elem() : _face_info->neighborPtr();
  const Elem * bulk_elem;
  if (!_use_wall_cell)
  {
    Point p = _face_info->faceCentroid();
    Point du = Point(MetaPhysicL::raw_value(_normal));
    du *= _bulk_distance;
    if (elemIsOne())
      p -= du;
    else
      p += du;
    bulk_elem = (*_pl)(p);
  }
  else
    bulk_elem = current_elem;

  mooseAssert(bulk_elem,
              "The element at bulk_distance from the wall was not found in the mesh. "
              "Increase the number of ghost layers with the 'ghost_layers' parameter.");
  mooseAssert(var1().hasBlocks(bulk_elem->subdomain_id()),
              "The fluid temperature is not defined at bulk_distance from the wall.");

  if (elemIsOne())
    return _htc_elem[_qp] *
           (_temp_fluid->getElemValue(bulk_elem) - _temp_solid->getBoundaryFaceValue(*_face_info));
  else
    return _htc_neighbor[_qp] *
           (_temp_fluid->getElemValue(bulk_elem) - _temp_solid->getBoundaryFaceValue(*_face_info));
}
