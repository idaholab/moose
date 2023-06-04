/*
Recompute Residual after System Solve
*/

#pragma once

#include "GeneralUserObject.h"

class ResidualEvaluationUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  ResidualEvaluationUserObject(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  std::set<TagID> _vector_tags;
};
