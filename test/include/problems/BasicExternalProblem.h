#pragma once

#include "ExternalProblem.h"

class BasicExternalProblem : public ExternalProblem
{
public:
  BasicExternalProblem(const InputParameters & params);
  ~BasicExternalProblem() = default;
  static InputParameters validParams();

  virtual void addExternalVariables() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

protected:
  unsigned int _heat_source_var;
};
