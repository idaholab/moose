#pragma once

#include "ExternalProblem.h"

class TransientExternalProblem : public ExternalProblem
{
public:
  TransientExternalProblem(const InputParameters & params);
  ~TransientExternalProblem() = default;
  static InputParameters validParams();

  virtual void addExternalVariables() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

protected:
  unsigned int _heat_source_var;
};
