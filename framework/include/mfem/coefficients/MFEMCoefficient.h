#pragma once

#include <mfem.hpp>
#include "MFEMGeneralUserObject.h"
#include "Function.h"

libMesh::Point PointFromMFEMVector(const mfem::Vector & vec);

class MFEMCoefficient : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMCoefficient(const InputParameters & parameters);
  virtual ~MFEMCoefficient();

  virtual std::shared_ptr<mfem::Coefficient> getCoefficient() const
  {
    mooseError("Base class MFEMCoefficient cannot return a valid Coefficient. Use a child class.");
  }
};
