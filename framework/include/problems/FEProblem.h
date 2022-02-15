//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblemBase.h"

class FEProblem;
class NonlinearSystem;

template <>
InputParameters validParams<FEProblem>();

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  FEProblem(const InputParameters & parameters);

  virtual bool getUseNonlinear() const { return _use_nonlinear; }
  virtual void setUseNonlinear(bool use_nonlinear) { _use_nonlinear = use_nonlinear; }

  virtual void setInputParametersFEProblem(InputParameters & parameters) override;

  NonlinearSystem & getNonlinearSystem() override { return *_nl_sys; }

  virtual void addLineSearch(const InputParameters & parameters) override;

protected:
  bool _use_nonlinear;
  std::shared_ptr<NonlinearSystem> _nl_sys;
};

using FVProblem = FEProblem;
