#pragma once

#include "ExternalProblem.h"

class NoSolveProblem;

class NoSolveProblem : public ExternalProblem
{
public:
  NoSolveProblem(const InputParameters & params);

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;

public:
  static InputParameters validParams();
};
