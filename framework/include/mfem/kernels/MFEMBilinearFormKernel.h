#pragma once

#include "MFEMGeneralUserObject.h"
#include "kernels.h"
#include "gridfunctions.h"

class MFEMBilinearFormKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBilinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMBilinearFormKernel() {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() = 0;
  mfem::Array<int> getMarkerArray() { return _elem_attr; };

  // Array listing element attributes integrator shall be applied on
  mfem::Array<int> _elem_attr;
};
