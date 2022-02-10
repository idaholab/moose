//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"

class CosineHumpFunctionTest : public MooseObjectUnitTest
{
public:
  CosineHumpFunctionTest()
    : MooseObjectUnitTest("ThermalHydraulicsApp"),
      _fn_name_positive("positive_hump"),
      _fn_name_negative("negative_hump"),
      _hump_center_position(3.0),
      _hump_width(2.0),
      _hump_begin_value(5.0),
      _hump_center_value_positive(7.0),
      _hump_center_value_negative(2.5)
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    // positive hump
    {
      const std::string class_name = "CosineHumpFunction";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MooseEnum>("axis") = "y";
      params.set<Real>("hump_center_position") = _hump_center_position;
      params.set<Real>("hump_width") = _hump_width;
      params.set<Real>("hump_begin_value") = _hump_begin_value;
      params.set<Real>("hump_center_value") = _hump_center_value_positive;
      _fe_problem->addFunction(class_name, _fn_name_positive, params);
    }

    // negative hump
    {
      const std::string class_name = "CosineHumpFunction";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MooseEnum>("axis") = "y";
      params.set<Real>("hump_center_position") = _hump_center_position;
      params.set<Real>("hump_width") = _hump_width;
      params.set<Real>("hump_begin_value") = _hump_begin_value;
      params.set<Real>("hump_center_value") = _hump_center_value_negative;
      _fe_problem->addFunction(class_name, _fn_name_negative, params);
    }
  }

  /// Tested function name for the "positive" hump tests
  const FunctionName _fn_name_positive;
  /// Tested function name for the "negative" hump tests
  const FunctionName _fn_name_negative;
  /// Hump center position
  const Real _hump_center_position;
  /// Hump width
  const Real _hump_width;
  /// Value before and after hump
  const Real _hump_begin_value;
  /// Value at center of hump for the "positive" hump tests
  const Real _hump_center_value_positive;
  /// Value at center of hump for the "negative" hump tests
  const Real _hump_center_value_negative;
};
