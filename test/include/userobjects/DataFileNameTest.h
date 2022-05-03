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
 * Tests MooseObject::getDataFileName()
 */
class DataFileNameTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  DataFileNameTest(const InputParameters & params);

  virtual void initialSetup(){};

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};
};
