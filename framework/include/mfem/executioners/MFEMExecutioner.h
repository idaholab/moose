#ifdef MFEM_ENABLED

#pragma once
#include "Executioner.h"
#include "MFEMProblemData.h"
#include "MFEMEstimator.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);

  virtual bool lastSolveConverged() const override { return true; };

  /// Virtual method to construct the ProblemOperator. Call for default problems.
  virtual void constructProblemOperator() = 0;

  // Executioners should not support estimators by default
  virtual bool addEstimator( std::shared_ptr<MFEMEstimator> ) { return false; }

  // Return false if it's time to stop
  virtual bool PRefine() {return false;};
  virtual bool HRefine() {return false;};
  virtual void UpdateAfterRefinement() {};

  /**
   * Set the device to use to solve the FE problem.
   */
  void setDevice();

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
  mfem::Device _device;
  bool _use_amr = false;
};

#endif
