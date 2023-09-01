//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorPositions.h"

registerMooseObject("MooseApp", FunctorPositions);

InputParameters
FunctorPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params += NonADFunctorInterface::validParams();

  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "positions_functors", "Functors providing the XYZ coordinates of the positions");
  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  // All functors defined on all processes for now
  params.set<bool>("auto_broadcast") = false;
  // Keep as up-to-date as possible given the generality of functors
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription(
      "Import positions from one or more reporters, for example other Positions");
  return params;
}

FunctorPositions::FunctorPositions(const InputParameters & parameters)
  : Positions(parameters), NonADFunctorInterface(this)

{
  const auto & functor_names = getParam<std::vector<MooseFunctorName>>("positions_functors");

  // Check input sizes
  if (functor_names.size() % 3 != 0)
    paramError("position_functors",
               "The list of functors must be divisible by three, the number of coordinates");

  for (const auto & name : functor_names)
    _pos_functors.push_back(&getFunctor<Real>(name));

  // Obtain the positions by evaluating the functors
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
FunctorPositions::initialize()
{
  clearPositions();
  const auto n_positions = _pos_functors.size() / 3;
  _positions.resize(n_positions);

  // Use the mesh center as a global argument for now
  _fe_problem.mesh().errorIfDistributedMesh(type());
  // Locate the origin on the mesh
  const Point p(0, 0, 0);
  auto pl = _fe_problem.mesh().getMesh().sub_point_locator();
  auto * const elem = (*pl)(p);
  if (!elem)
    mooseError("Origin point not in local mesh, cannot evaluate the functor there");
  const Moose::ElemPointArg elem_origin = {elem, p, false};
  const auto t = determineState();

  for (auto i : make_range(n_positions))
    _positions[i] = {(*_pos_functors[3 * i])(elem_origin, t),
                     (*_pos_functors[3 * i + 1])(elem_origin, t),
                     (*_pos_functors[3 * i + 2])(elem_origin, t)};
}
