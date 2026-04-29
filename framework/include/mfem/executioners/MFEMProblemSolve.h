//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "ProblemOperatorBase.h"
#include "SolveObject.h"

class Executioner;

namespace Moose::MFEM
{
class Problem;

class ProblemSolve : public SolveObject
{
public:
  static InputParameters validParams();

  ProblemSolve(Executioner & ex,
               std::vector<std::shared_ptr<ProblemOperatorBase>> & problem_operators);

  /**
   * Solve routine provided by this object.
   * @return True if solver is converged. Currently, this always will return true for
   * Moose::MFEM::ProblemSolve.
   */
  virtual bool solve() override;

protected:
  Problem & _mfem_problem;
  std::vector<std::shared_ptr<ProblemOperatorBase>> & _problem_operators;
};

} // namespace Moose::MFEM
#endif
