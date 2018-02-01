//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PPBINDINGSTEADY_H
#define PPBINDINGSTEADY_H

#include "Steady.h"

class PPBindingSteady;

template <>
InputParameters validParams<PPBindingSteady>();

/**
 * Steady excecutioner testing postprocessor binding
 */
class PPBindingSteady : public Steady
{
public:
  PPBindingSteady(const InputParameters & parameters);

  virtual void init() override;

protected:
  const PostprocessorValue & _pp;
  const PostprocessorValue & _pp_old;
  const PostprocessorValue & _pp_older;
};

#endif /* PPBINDINGSTEADY_H */
