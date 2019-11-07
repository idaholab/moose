//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"

/**
 * Steady excecutioner testing postprocessor binding
 */
class PPBindingSteady : public Steady
{
public:
  static InputParameters validParams();

  PPBindingSteady(const InputParameters & parameters);

  virtual void init() override;

protected:
  const PostprocessorValue & _pp;
  const PostprocessorValue & _pp_old;
  const PostprocessorValue & _pp_older;
};
