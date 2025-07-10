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

#include "ProblemOperator.h"
#include "EquationSystemInterface.h"
#include "MFEMEstimator.h"
#include "MFEMThresholdRefiner.h"

namespace Moose::MFEM
{
/// Steady-state problem operator with an equation system.
class EquationSystemProblemOperator : public ProblemOperator, public EquationSystemInterface
{
public:
  EquationSystemProblemOperator(MFEMProblem & problem)
    : ProblemOperator(problem), _equation_system(_problem_data.eqn_system)
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;
  virtual void Solve() override;

  //! Call this with the parameters for the Estimator
  void AddEstimator(std::shared_ptr<MFEMEstimator> estimator) override;
  void AddRefiner(std::shared_ptr<MFEMThresholdRefiner> refiner) override;
  void SetUpAMR() override;
  bool HRefine() override;
  bool PRefine() override;

  bool UseHRefinement() const override { return _estimator and _refiner and _refiner->UseHRefinement(); }
  bool UsePRefinement() const override { return _estimator and _refiner and _refiner->UsePRefinement(); }

  ~EquationSystemProblemOperator() override = default;

  [[nodiscard]] Moose::MFEM::EquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added to ProblemOperator.");
    }

    return _equation_system.get();
  }

private:
  bool _use_amr{false};
  std::shared_ptr<Moose::MFEM::EquationSystem> _equation_system{nullptr};
<<<<<<< HEAD
  std::shared_ptr<MFEMEstimator>               _estimator;
  std::unique_ptr<mfem::ThresholdRefiner>      _refiner;
=======
  std::shared_ptr<MFEMEstimator> _estimator;
  std::shared_ptr<MFEMThresholdRefiner> _refiner;

  /**
   * For now, use a bool to determine whether we use amr
   */
  bool _use_amr;
>>>>>>> 2ee1fa2fb2 (Add refiner class and modify h/p refinement control flow)
};

} // namespace Moose::MFEM

#endif
