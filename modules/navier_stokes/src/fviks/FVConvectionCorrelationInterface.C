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
    _pl(mesh().getPointLocator()),
    _var1_is_fluid("wraps_" + var1().name() == _temp_fluid.functorName())
{
  if (!_use_wall_cell && (_bulk_distance < 0))
    mooseError(
        "The bulk distance should be specified or 'wall_cell_is_bulk' should be set to true for "
        "the FVTwoVarConvectionCorrelationInterface");
}

ADReal
FVConvectionCorrelationInterface::computeQpResidual()
{
  // If variable1 is fluid and variable 1 is on elem or
  // if variable2 is fluid and variable 2 is on elem
  // the fluid element will be elem otherwise it is the neighbor
  const Elem * elem_on_fluid_side =
      (elemIsOne() && _var1_is_fluid) || (!elemIsOne() && !_var1_is_fluid)
          ? &_face_info->elem()
          : _face_info->neighborPtr();

  const Elem * bulk_elem;
  const auto state = determineState();
  if (!_use_wall_cell)
  {
    Point p = _face_info->faceCentroid();
    Point du = Point(MetaPhysicL::raw_value(_normal));
    du *= _bulk_distance;
    // The normal always points outwards from the elem (towards the neighbor)
    if (elem_on_fluid_side == &_face_info->elem())
      p -= du;
    else
      p += du;
    bulk_elem = (*_pl)(p);
  }
  else
    bulk_elem = elem_on_fluid_side;

  mooseAssert(bulk_elem,
              "The element at bulk_distance from the wall was not found in the mesh. "
              "Increase the number of ghost layers with the 'ghost_layers' parameter.");
  mooseAssert((_var1_is_fluid ? var1() : var2()).hasBlocks(bulk_elem->subdomain_id()),
              "The fluid temperature is not defined at bulk_distance from the wall.");

  const auto fluid_side = singleSidedFaceArg(_var1_is_fluid ? var1() : var2(), _face_info);
  const auto solid_side = singleSidedFaceArg(_var1_is_fluid ? var2() : var1(), _face_info);

  const auto bulk_elem_arg = makeElemArg(bulk_elem);

  /// We make sure that the gradient*normal part is addressed
  auto multipler =
      _normal * (_face_info->faceCentroid() - bulk_elem->vertex_average()) > 0 ? 1 : -1;

  return multipler * _htc(fluid_side, state) *
         (_temp_fluid(bulk_elem_arg, state) - _temp_solid(solid_side, state));
}
