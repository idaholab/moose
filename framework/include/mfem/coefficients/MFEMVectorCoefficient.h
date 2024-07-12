#pragma once

#include "MFEMGeneralUserObject.h"
#include "coefficients.h"
#include "Function.h"

class MFEMVectorCoefficient : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMVectorCoefficient(const InputParameters & parameters);
  virtual ~MFEMVectorCoefficient();

  virtual std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficient() const
  {
    mooseError("Base class MFEMVectorCoefficient cannot return a valid VectorCoefficient. Use a "
               "child class.");
  }
};
