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

#include "PointValue.h"
#include "Function.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PointValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this postprocessor operates on");
  params.addRequiredParam<Point>("point", "A point in space to be given to the function");
  return params;
}

PointValue::PointValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _mesh(_subproblem.mesh()),
    _point(getParam<Point>("point")),
    _point_vec(1, _point)
{
}

PointValue::~PointValue()
{
}

void
PointValue::initialize()
{
}

void
PointValue::execute()
{
  AutoPtr<PointLocatorBase> pl = _mesh.getMesh().sub_point_locator();

  // First find the element the hit lands in
  const Elem * elem = (*pl)(_point);

  mooseAssert(elem, "Element not found!");

  _subproblem.reinitElemPhys(elem, _point_vec, 0);
}

Real
PointValue::getValue()
{
  mooseAssert(_u.size() == 1, "No values in u!");
  return _u[0];
}
