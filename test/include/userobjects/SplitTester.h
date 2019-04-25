//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralUserObject.h"

// Forward Declarations
class SplitTester;

template <>
InputParameters validParams<SplitTester>();

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class SplitTester : public GeneralUserObject
{
public:
  SplitTester(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override{};
  virtual void finalize() override{};
};

