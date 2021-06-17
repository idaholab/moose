//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * A UserObject that tests the requesting of Reporter values
 * that are actually declared later to be Postprocessor
 * and VectorPostprocessor values.
 */
class ReporterSpecialTypeTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ReporterSpecialTypeTest(const InputParameters & params);

  void initialSetup() override;
  void initialize() override{};
  void execute() override{};
  void finalize() override{};

private:
  bool isPostprocessor(const std::string & param_name) const;
  bool isVectorPostprocessor(const std::string & param_name) const;
};
