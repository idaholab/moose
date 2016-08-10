/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FindValueOnLine.h"
#include "MooseMesh.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<FindValueOnLine>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription("Find a specific target value along a sampling line. The variable values along the line should change monotonically. The target value is searched using a bisection algorithm.");
  params.addParam<Point>("start_point", "Start point of the sampling line.");
  params.addParam<Point>("end_point", "End point of the sampling line.");
  params.addParam<Real>("target", "Target value to locate.");
  params.addParam<unsigned int>("depth", 30, "Maximum number of bisections to perform.");
  params.addParam<Real>("tol", 1e-10, "Stop search if a value is found that is equal to the target with this tolerance applied.");
  params.addCoupledVar("v", "Variable to inspect");
  return params;
}

FindValueOnLine::FindValueOnLine(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    Coupleable(this, false),
    _start_point(getParam<Point>("start_point")),
    _end_point(getParam<Point>("end_point")),
    _length((_end_point - _start_point).norm()),
    _target(getParam<Real>("target")),
    _depth(getParam<unsigned int>("depth")),
    _tol(getParam<Real>("tol")),
    _coupled_var(getVar("v", 0)),
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
}

void
FindValueOnLine::execute()
{
  Real s;
  Real s_left = 0.0;
  Real left = getValueAtPoint(_start_point);
  Real s_right = 1.0;
  Real right = getValueAtPoint(_end_point);

  for (unsigned i = 0; i < _depth; ++i)
  {
    // find midpoint
    s = (s_left + s_right) / 2.0;
    Point p = s * (_end_point - _start_point) + _start_point;

    // sample value
    Real value = getValueAtPoint(p);

    // have we hit the target value yet?
    if (MooseUtils::absoluteFuzzyEqual(value, _target, _tol))
      break;

    // bisect
    if (   (left >= _target && _target >= value)
        || (left <= _target && _target <= value))
    {
      // to the left
      s_right = s;
      right = value;
    }
    else
    {
      // to the right
      s_left = s;
      left = value;
    }
  }

  _position = s * _length;
}

Real
FindValueOnLine::getValueAtPoint(const Point & p)
{
  const Elem * elem = (*_pl)(p);

  if (elem)
  {
    Real value;

    if (elem->processor_id() == processor_id())
    {
      // element is local
      _point_vec[0] = p;
      _subproblem.reinitElemPhys(elem, _point_vec, 0);
      value = _coupled_var->sln()[0];
    }

    // broadcast value
    _communicator.broadcast(value, elem->processor_id());
    return value;
  }
  else
  {
    // there is no element
    mooseError("No element found at the current search point. Please make sure the sampling line stays inside the mesh completely.");
  }
}

PostprocessorValue
FindValueOnLine::getValue()
{
  return _position;
}
