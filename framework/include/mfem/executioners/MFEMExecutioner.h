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

  /// Returns a pointer to the operator. See derived classes.
  [[nodiscard]] virtual mfem::Operator * GetOperator() const = 0;

  /// Virtual method to construct the operator. Call for default problems.
  virtual void ConstructOperator() = 0;

  /**
   * Set the device to use to solve the FE problem.
   */
  void setDevice();

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
  mfem::Device _device;
};
