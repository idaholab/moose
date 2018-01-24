//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NUMRESIDUALEVALUATIONS_H
#define NUMRESIDUALEVALUATIONS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumResidualEvaluations;

template <>
InputParameters validParams<NumResidualEvaluations>();

/**
 * Just returns the total number of Residual Evaluations performed.
 */
class NumResidualEvaluations : public GeneralPostprocessor
{
public:
  NumResidualEvaluations(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;
};

#endif // NUMRESIDUALEVALUATIONS_H
