//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterWrapper1PhaseProblem.h"

class QuadInterWrapper1PhaseProblem;

/**
 * Quadrilateral interwrapper solver
 */
class QuadInterWrapper1PhaseProblem : public InterWrapper1PhaseProblem
{
public:
  QuadInterWrapper1PhaseProblem(const InputParameters & params);

protected:
  virtual Real computeFrictionFactor(Real Re) override;

public:
  static InputParameters validParams();
};
