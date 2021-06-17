//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * This initial condition takes values from post-processor values.
 */
class PostprocessorIC : public InitialCondition
{
public:
  static InputParameters validParams();

  PostprocessorIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  const PostprocessorValue & _pp1;
  const PostprocessorValue & _pp2;
};
