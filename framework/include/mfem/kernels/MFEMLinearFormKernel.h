#pragma once

#include "MFEMGeneralUserObject.h"
#include "kernels.h"
#include "gridfunctions.h"

/*
Class to construct a mfem::LinearFormIntegrator to apply to the
equation system.

TODO: Support for marker arrays specifying the block each kernel is applied on.
*/
class MFEMLinearFormKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMLinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMLinearFormKernel() {}

  virtual mfem::LinearFormIntegrator * createIntegrator() = 0;
};