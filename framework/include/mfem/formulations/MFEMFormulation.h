#pragma once

#include "MFEMGeneralUserObject.h"
#include "problem_builder_base.h"

class MFEMFormulation : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMFormulation(const InputParameters & parameters);
  virtual ~MFEMFormulation() override = default;

  virtual std::shared_ptr<platypus::ProblemBuilder> getProblemBuilder() const
  {
    mooseError(
        "Base class MFEMFormulation cannot return a valid ProblemBuilder. Use a child class.");
  }
};
