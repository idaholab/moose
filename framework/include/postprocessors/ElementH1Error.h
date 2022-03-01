//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementW1pError.h"

/**
 * This postprocessor will print out the H^1-norm of the difference
 * between the computed solution and the passed function, where the
 * norm is defined as:
 *
 * ||u-f||_{H^1} = sqrt( \int ( |u-f|^2 + |grad u - grad f|^2 ) dx )
 */
class ElementH1Error : public ElementW1pError
{
public:
  static InputParameters validParams();

  ElementH1Error(const InputParameters & parameters);
};
