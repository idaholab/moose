//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  const Elem * elem_on_fluid_side = _var1_is_fluid ? &elem1() : &elem2();

  const Elem * bulk_elem;
  const auto state = determineState();
  if (!_use_wall_cell)
  {
    Point p = _face_info->faceCentroid();
    const Point du = normal() * _bulk_distance;
    if (_var1_is_fluid)
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

  const auto fluid_side = _var1_is_fluid ? faceArg1() : faceArg2();
  const auto solid_side = _var1_is_fluid ? faceArg2() : faceArg1();

  const auto bulk_elem_arg = makeElemArg(bulk_elem);

  return (_var1_is_fluid ? 1 : -1) * _htc(fluid_side, state) *
         (_temp_fluid(bulk_elem_arg, state) - _temp_solid(solid_side, state));
}
