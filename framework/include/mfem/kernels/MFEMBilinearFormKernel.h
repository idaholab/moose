#pragma once

#include "MFEMGeneralUserObject.h"
#include "gridfunctions.h"

/*
Class to construct a mfem::BilinearFormIntegrator to apply to the
equation system.

TODO: Support for marker arrays specifying the block each kernel is applied on.
*/
class MFEMBilinearFormKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBilinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMBilinearFormKernel() {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() = 0;
};
