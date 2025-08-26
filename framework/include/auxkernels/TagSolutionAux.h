//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TagResidualAux.h"

/**
 * The value of a tagged solution vector for a given variable is coupled to
 * the current AuxVariable.
 */
class TagSolutionAux : public TagResidualAux
{
public:
  static InputParameters validParams();

  TagSolutionAux(const InputParameters & parameters);

  void initialSetup() override;
};
