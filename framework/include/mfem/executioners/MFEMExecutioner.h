#pragma once
#include "Executioner.h"
#include "MFEMProblemData.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);
  virtual ~MFEMExecutioner() override = default;

  virtual bool lastSolveConverged() const override { return true; };

  /// Virtual method to construct the ProblemOperator. Call for default problems.
  virtual void constructProblemOperator() = 0;

  /**
   * Set the device to use to solve the FE problem.
   */
  void setDevice();

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
  mfem::Device _device;
};
