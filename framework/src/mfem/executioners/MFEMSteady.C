//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSteady.h"
#include "MFEMProblem.h"
#include "MFEMEigenproblem.h"
#include "EigenproblemEquationSystem.h"
#include "EquationSystemProblemOperator.h"
#include "EigenproblemESProblemOperator.h"

registerMooseObject("MooseApp", MFEMSteady);

InputParameters
MFEMSteady::validParams()
{
  InputParameters params = MFEMProblemSolve::validParams();
  params.addClassDescription("Executioner for steady state MFEM problems.");
  params += SteadyBase::validParams();
  return params;
}

MFEMSteady::MFEMSteady(const InputParameters & params)
  : SteadyBase(params),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _mfem_problem_data(_mfem_problem.getProblemData()),
    _mfem_problem_solve(*this, getProblemOperators())
{
  _fixed_point_solve->setInnerSolve(_mfem_problem_solve);
  // If no ProblemOperators have been added by the user, add a default
  if (getProblemOperators().empty())
  {
    if (_mfem_problem.getNumericType() == MFEMProblem::NumericType::REAL)
    {
      if (dynamic_cast<MFEMEigenproblem *>(&_mfem_problem))
      {
        _mfem_problem_data.eqn_system = std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
        auto problem_operator =
            std::make_shared<Moose::MFEM::EigenproblemESProblemOperator>(_mfem_problem);
        addProblemOperator(std::move(problem_operator));
      }
      else
      {
        _mfem_problem_data.eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
        auto problem_operator =
            std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(_mfem_problem);
        addProblemOperator(std::move(problem_operator));
      }
    }
    else if (_mfem_problem.getNumericType() == MFEMProblem::NumericType::COMPLEX)
    {
      _mfem_problem_data.eqn_system = std::make_shared<Moose::MFEM::ComplexEquationSystem>();
      auto problem_operator =
          std::make_shared<Moose::MFEM::ComplexEquationSystemProblemOperator>(_mfem_problem);
      addProblemOperator(std::move(problem_operator));
    }
    else
      mooseError("Unknown numeric type. "
                 "Please set the Problem numeric type to either 'real' or 'complex'.");
  }
}

void
MFEMSteady::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

  if (_mfem_problem_data.nonlinear_solver)
    _mfem_problem_data.eqn_system->SetSolverRequiresGradient(
        _mfem_problem_data.nonlinear_solver->RequiresGradient());

  // Set up initial conditions
  _mfem_problem_data.eqn_system->Init(
      _mfem_problem_data.gridfunctions,
      _mfem_problem_data.cmplx_gridfunctions,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  for (const auto & problem_operator : getProblemOperators())
  {
    problem_operator->SetGridFunctions();
    problem_operator->Init(_mfem_problem_data.f);
  }
}

#endif
