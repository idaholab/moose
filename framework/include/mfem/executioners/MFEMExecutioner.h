#pragma once
#include "Executioner.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);
  virtual ~MFEMExecutioner() override = default;

  // Solve the current system of equations
  virtual void Solve() const = 0;

  // Execute solution strategy including any timestepping
  virtual void Execute() const = 0;

  virtual void execute() override{};

  virtual bool lastSolveConverged() const override { return true; };

protected:
  MFEMProblem & _mfem_problem;
};
