#pragma once
#include "Executioner.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);
  virtual ~MFEMExecutioner() override = default;

  virtual bool lastSolveConverged() const override { return true; };

protected:
  MFEMProblem & _mfem_problem;
};
