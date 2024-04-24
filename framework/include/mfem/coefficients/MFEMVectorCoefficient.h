#pragma once

#include "GeneralUserObject.h"
#include "coefficients.hpp"
#include "Function.h"

class MFEMVectorCoefficient : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMVectorCoefficient(const InputParameters & parameters);
  virtual ~MFEMVectorCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  virtual std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficient() const
  {
    mooseError("Base class MFEMVectorCoefficient cannot return a valid VectorCoefficient. Use a "
               "child class.");
  }
};
