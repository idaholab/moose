//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FindValueOnLine.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseVariable.h"

registerMooseObject("MooseApp", FindValueOnLine);

InputParameters
FindValueOnLine::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Find a specific target value along a sampling line. The variable "
                             "values along the line should change monotonically. The target value "
                             "is searched using a bisection algorithm.");
  params.addParam<Point>("start_point", "Start point of the sampling line.");
  params.addParam<Point>("end_point", "End point of the sampling line.");
  params.addParam<Real>("target", "Target value to locate.");
  params.addParam<bool>(
      "error_if_not_found",
      true,
      "If true, stop with error if target value is not found on the line. If false, "
      "return default_value.");
  params.addParam<Real>("default_value",
                        -1,
                        "Value to return if target value is not found on line and "
                        "error_if_not_found is false.");
  params.addParam<unsigned int>("depth", 36, "Maximum number of bisections to perform.");
  params.addParam<Real>(
      "tol",
      1e-10,
      "Stop search if a value is found that is equal to the target with this tolerance applied.");
  params.addCoupledVar("v", "Variable to inspect");
  return params;
}

FindValueOnLine::FindValueOnLine(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    Coupleable(this, false),
    _start_point(getParam<Point>("start_point")),
    _end_point(getParam<Point>("end_point")),
    _length((_end_point - _start_point).norm()),
    _target(getParam<Real>("target")),
    _error_if_not_found(getParam<bool>("error_if_not_found")),
    _default_value(getParam<Real>("default_value")),
    _depth(getParam<unsigned int>("depth")),
    _tol(getParam<Real>("tol")),
    _coupled_var(*getVar("v", 0)),
    _position(0.0),
    _mesh(_subproblem.mesh()),
    _point_vec(1)
{
}

void
FindValueOnLine::initialize()
{
  // We do this here just in case it's been destroyed and recreated becaue of mesh adaptivity.
  _pl = _mesh.getPointLocator();
  _pl->enable_out_of_mesh_mode();
}

void
FindValueOnLine::execute()
{
  Real s;
  Real s_left = 0.0;
  Real left = getValueAtPoint(_start_point);
  Real s_right = 1.0;
  Real right = getValueAtPoint(_end_point);

  /**
   * Here we determine the direction of the solution. i.e. the left might be the high value
   * while the right might be the low value.
   */
  bool left_to_right = left < right;
  // Initial bounds check
  if ((left_to_right && _target < left) || (!left_to_right && _target < right))
  {
    if (_error_if_not_found)
    {
      mooseError("Target value \"",
                 _target,
                 "\" is less than the minimum sampled value \"",
                 std::min(left, right),
                 "\"");
    }
    else
    {
      _position = _default_value;
      return;
    }
  }
  if ((left_to_right && _target > right) || (!left_to_right && _target > left))
  {
    if (_error_if_not_found)
    {
      mooseError("Target value \"",
                 _target,
                 "\" is greater than the maximum sampled value \"",
                 std::max(left, right),
                 "\"");
    }
    else
    {
      _position = _default_value;
      return;
    }
  }

  bool found_it = false;
  Real value = 0;
  for (unsigned int i = 0; i < _depth; ++i)
  {
    // find midpoint
    s = (s_left + s_right) / 2.0;
    Point p = s * (_end_point - _start_point) + _start_point;

    // sample value
    value = getValueAtPoint(p);

    // have we hit the target value yet?
    if (MooseUtils::absoluteFuzzyEqual(value, _target, _tol))
    {
      found_it = true;
      break;
    }

    // bisect
    if ((left_to_right && _target < value) || (!left_to_right && _target > value))
      // to the left
      s_right = s;
    else
      // to the right
      s_left = s;
  }

  // Return error if target value (within tol) was not found within depth bisections
  if (!found_it)
    mooseError("Target value \"",
               std::setprecision(10),
               _target,
               "\" not found on line within tolerance, last sample: ",
               value,
               ".");

  _position = s * _length;
}

Real
FindValueOnLine::getValueAtPoint(const Point & p)
{
  const Elem * elem = (*_pl)(p);

  processor_id_type elem_proc_id =
      elem ? elem->processor_id() : libMesh::DofObject::invalid_processor_id;
  _communicator.min(elem_proc_id);

  if (elem_proc_id == libMesh::DofObject::invalid_processor_id)
  {
    // there is no element
    mooseError("No element found at the current search point. Please make sure the sampling line "
               "stays inside the mesh completely.");
  }

  Real value = 0;

  if (elem)
  {
    if (elem->processor_id() == processor_id())
    {
      // element is local
      _point_vec[0] = p;
      _subproblem.reinitElemPhys(elem, _point_vec, 0);
      value = _coupled_var.sln()[0];
    }
  }

  // broadcast value
  _communicator.broadcast(value, elem_proc_id);
  return value;
}

PostprocessorValue
FindValueOnLine::getValue() const
{
  return _position;
}
