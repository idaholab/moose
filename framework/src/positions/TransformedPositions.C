//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransformedPositions.h"
#include "GeometryUtils.h"

registerMooseObject("MooseApp", TransformedPositions);

InputParameters
TransformedPositions::validParams()
{
  InputParameters params = Positions::validParams();

  params.addRequiredParam<PositionsName>("base_positions",
                                         "Positions object providing the positions to transform");

  MooseEnum transforms("TRANSLATE ROTATE_XYZ SCALE");
  params.addRequiredParam<MooseEnum>(
      "transform",
      transforms,
      "The type of transformation to perform (TRANSLATE, ROTATE_XYZ, SCALE)");
  params.addParam<RealVectorValue>(
      "vector_value",
      "The value to use for the transformation. When using TRANSLATE or SCALE, the "
      "xyz coordinates are applied in each direction respectively. When using "
      "ROTATE_XYZ, the values are interpreted as rotations in degrees around the X, Y and Z axis, "
      "in that order.");

  // Use base position ordering
  params.set<bool>("auto_sort") = false;
  // If the base position is broadcast already we do not need to
  params.set<bool>("auto_broadcast") = false;
  // Keep as up-to-date as possible given that the base position could be changing
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription(
      "Transform, with a linear transformation, positions from another Positions object");
  return params;
}

TransformedPositions::TransformedPositions(const InputParameters & parameters)
  : Positions(parameters),
    _transform(getParam<MooseEnum>("transform")),
    _vector_value(getParam<RealVectorValue>("vector_value"))
{
  const auto & base_name = getParam<PositionsName>("base_positions");
  if (_fe_problem.hasUserObject(base_name))
    _base_positions = &_fe_problem.getPositionsObject(base_name);
  else
    mooseError("Base positions has not been created yet. If it exists, re-order Positions objects "
               "in the input file or implement automated construction ordering for Positions");

  // Obtain the positions from the base, then transform them
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
TransformedPositions::initialize()
{
  clearPositions();
  const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
  if (!_base_positions->initialized(initial))
    mooseError("Positions '", _base_positions->name(), "' is not initialized.");

  const auto n_positions = _base_positions->getNumPositions(initial);
  _positions.resize(n_positions);

  for (const auto i : make_range(n_positions))
  {
    const auto & base_point = _base_positions->getPosition(i, initial);
    if (_transform == "ROTATE_XYZ")
      _positions[i] = geom_utils::rotatePointAboutAxis(
          geom_utils::rotatePointAboutAxis(
              geom_utils::rotatePointAboutAxis(
                  base_point, _vector_value(0) / 90 * M_PI_2, Point(1, 0, 0)),
              _vector_value(1) / 90 * M_PI_2,
              Point(0, 1, 0)),
          _vector_value(2) / 90 * M_PI_2,
          Point(0, 0, 1));
    else if (_transform == "SCALE")
      _positions[i] = Point(base_point(0) * _vector_value(0),
                            base_point(1) * _vector_value(1),
                            base_point(2) * _vector_value(2));
    else
      _positions[i] = Point(base_point(0) + _vector_value(0),
                            base_point(1) + _vector_value(1),
                            base_point(2) + _vector_value(2));

    _initialized = true;
  }
}
