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

#include "SteadyBase.h"
#include "ProblemOperatorInterface.h"
#include "MFEMProblemSolve.h"
#include "EquationSystemProblemOperator.h"
#include "ComplexEquationSystemProblemOperator.h"

class MFEMSteady : public SteadyBase, public Moose::MFEM::ProblemOperatorInterface
{
public:
  static InputParameters validParams();

  explicit MFEMSteady(const InputParameters & params);

  virtual void init() override;

private:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _mfem_problem_data;
  MFEMProblemSolve _mfem_problem_solve;
};

#endif
