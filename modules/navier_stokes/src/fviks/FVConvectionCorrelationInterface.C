//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVConvectionCorrelationInterface.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVConvectionCorrelationInterface);

InputParameters
FVConvectionCorrelationInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription("Computes the residual for a convective heat transfer across an "
                             "interface for the finite volume method, "
                             "using a correlation for the heat transfer coefficient.");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "The fluid temperature variable");
  params.addRequiredParam<MooseFunctorName>(NS::T_solid, "The solid/wall temperature variable");
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient");
  params.addParam<Real>(
      "bulk_distance", -1, "The distance to the bulk for evaluating the fluid bulk temperature");
  params.addParam<bool>("wall_cell_is_bulk",
                        false,
                        "Use the wall cell centroid temperature for the fluid bulk temperature");
  return params;
}

FVConvectionCorrelationInterface::FVConvectionCorrelationInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _temp_fluid(getFunctor<ADReal>(NS::T_fluid)),
    _temp_solid(getFunctor<ADReal>(NS::T_solid)),
    _htc(getFunctor<ADReal>("h")),
    _bulk_distance(getParam<Real>("bulk_distance")),
    _use_wall_cell(getParam<bool>("wall_cell_is_bulk")),
    _pl(mesh().getPointLocator())
{
  if (!_use_wall_cell && (_bulk_distance < 0))
    mooseError(
        "The bulk distance should be specified or 'wall_cell_is_bulk' should be set to true for "
        "the FVTwoVarConvectionCorrelationInterface");
  if ("wraps_" + var1().name() != _temp_fluid.functorName())
    paramError(NS::T_fluid, "variable1 must be equal to T_fluid parameter.");
  if ("wraps_" + var2().name() != _temp_solid.functorName())
    paramError(NS::T_solid, "variable2 must be equal to T_solid parameter.");
}

ADReal
FVConvectionCorrelationInterface::computeQpResidual()
{
  const Elem * current_elem = elemIsOne() ? &_face_info->elem() : _face_info->neighborPtr();
  const Elem * bulk_elem;
  const auto state = determineState();
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

  const auto face_arg_side1 = singleSidedFaceArg(var1(), _face_info);
  const auto face_arg_side2 = singleSidedFaceArg(var2(), _face_info);
  const auto bulk_elem_arg = makeElemArg(bulk_elem);

  /// We make sure that the gradient*normal part is addressed
  auto multipler =
      _normal * (_face_info->faceCentroid() - bulk_elem->vertex_average()) > 0 ? 1 : -1;

  return multipler * _htc(face_arg_side1, state) *
         (_temp_fluid(bulk_elem_arg, state) - _temp_solid(face_arg_side2, state));
}
