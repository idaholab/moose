#pragma once

#include "GeneralUserObject.h"
#include "coefficients.hpp"
#include "Function.h"

libMesh::Point PointFromMFEMVector(const mfem::Vector & vec);

class MFEMCoefficient : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMCoefficient(const InputParameters & parameters);
  virtual ~MFEMCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  virtual std::shared_ptr<mfem::Coefficient> getCoefficient() const
  {
    mooseError("Base class MFEMCoefficient cannot return a valid Coefficient. Use a child class.");
  }
};
