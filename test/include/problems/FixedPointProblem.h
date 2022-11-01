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

/**
 * FEProblem derived class for lagging a tagged residual.
 */
class FixedPointProblem : public FEProblem
{
public:
  static InputParameters validParams();

  FixedPointProblem(const InputParameters & params);

  virtual void computeResidual(const NumericVector<Number> & soln,
                               NumericVector<Number> & residual,
                               unsigned int nl_sys_num = 0) override;
  virtual void computeFullResidual(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual);

  bool taggedVectorForResidual() const { return _tagged_vector_for_partial_residual; }

  void copySolution();

protected:
  const bool _tagged_vector_for_partial_residual;
  const TagName _tag_previous;
  const TagID _tag_id;
  NumericVector<Number> & _tagged_vector;
};
