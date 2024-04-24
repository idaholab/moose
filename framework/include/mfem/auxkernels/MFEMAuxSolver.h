#pragma once

#include "GeneralUserObject.h"
#include "auxsolvers.hpp"
#include "gridfunctions.hpp"

class MFEMAuxSolver : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMAuxSolver(const InputParameters & parameters);
  virtual ~MFEMAuxSolver();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  inline virtual std::shared_ptr<hephaestus::AuxSolver> getAuxSolver() const { return _auxsolver; }

  virtual void storeCoefficients(hephaestus::Coefficients & coefficients) {}

protected:
  std::shared_ptr<hephaestus::AuxSolver> _auxsolver{nullptr};
};
