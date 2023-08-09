//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIDTest.h"

registerMooseObject("MooseTestApp", ElementIDTest);

InputParameters
ElementIDTest::validParams()
{
  InputParameters params = ElementReporter::validParams();
  params.addRequiredParam<ExtraElementIDName>("id_name1", "Name of an extra element id");
  params.addRequiredParam<ExtraElementIDName>("id_name2", "Name of an extra element id");
  return params;
}

ElementIDTest::ElementIDTest(const InputParameters & params)
  : ElementReporter(params),
    _id_name1(getParam<ExtraElementIDName>("id_name1")),
    _id_name2(getParam<ExtraElementIDName>("id_name2")),
    _mapping(declareValueByName<std::unordered_map<dof_id_type, std::set<dof_id_type>>>(
        _id_name1 + "_to_" + _id_name2, REPORTER_MODE_REPLICATED))
{
  _mapping = getElemIDMapping(_id_name1, _id_name2);
}
