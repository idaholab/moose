#pragma once

#include "GeneralUserObject.h"
#include "coefficients.h"
#include "auxsolver_base.h"

class MFEMAuxSolver : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMAuxSolver(const InputParameters & parameters);
  virtual ~MFEMAuxSolver();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  inline virtual std::shared_ptr<platypus::AuxSolver> getAuxSolver() const { return _auxsolver; }

  virtual void storeCoefficients(Coefficients & coefficients) {}

protected:
  std::shared_ptr<platypus::AuxSolver> _auxsolver{nullptr};
};
