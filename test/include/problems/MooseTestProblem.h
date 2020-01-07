//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

class AuxiliarySystem;

/**
 * FEProblemBase derived class for customization of callbacks. In this instance we only print out
 * something in the c-tor and d-tor, so we know the class was build and used properly.
 */
class MooseTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  MooseTestProblem(const InputParameters & params);
  virtual ~MooseTestProblem();

private:
  std::shared_ptr<AuxiliarySystem> _test_aux;
};
