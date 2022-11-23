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

class NonlinearSystem;

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

  NonlinearSystem & getNonlinearSystem(unsigned int nl_sys_num = 0) override;

  virtual void addLineSearch(const InputParameters & parameters) override;

protected:
  bool _use_nonlinear;
  std::vector<std::shared_ptr<NonlinearSystem>> _nl_sys;

private:
  using FEProblemBase::_nl;
};

using FVProblem = FEProblem;

inline NonlinearSystem &
FEProblem::getNonlinearSystem(const unsigned int nl_sys_num)
{
  return *_nl_sys[nl_sys_num];
}
