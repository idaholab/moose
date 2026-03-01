//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VariableResidualNormBase.h"

/**
 * Computes the residual norm (absolute value) of a NodeElem residual for a variable.
 */
class NodeElemVariableResidualNorm : public VariableResidualNormBase
{
public:
  static InputParameters validParams();

  NodeElemVariableResidualNorm(const InputParameters & parameters);

protected:
  virtual std::vector<dof_id_type> getCurrentElemDofIndices() const override;
  virtual void computeNorm() override;
};
