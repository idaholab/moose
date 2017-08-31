/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ExtraQPTest.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ExtraQPTest>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Set up a list of discrete point to evaluate material properties on "
                             "using the XFEMMaterialManager");
  params.addRequiredParam<std::vector<Point>>("points", "List of extra QP points");
  return params;
}

ExtraQPTest::ExtraQPTest(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    ExtraQPProvider(),
    _mesh(_fe_problem.mesh()),
    _points(getParam<std::vector<Point>>("points"))
{
}

void
ExtraQPTest::initialSetup()
{
  auto pl = _mesh.getPointLocator();

  for (auto point : _points)
  {
    auto elem = (*pl)(point);
    if (elem && elem->processor_id() == processor_id())
      _extra_qp_map[elem->id()].push_back(point);
  }
}
