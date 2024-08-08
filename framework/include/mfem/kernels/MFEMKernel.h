#pragma once

#include "MFEMGeneralUserObject.h"
#include "gridfunctions.h"

/*
Class to construct an MFEM integrator to apply to the
equation system.

TODO: Support for marker arrays specifying the block each kernel is applied on.
*/

template <typename T>
class MFEMKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams()
  {
    InputParameters params = MFEMGeneralUserObject::validParams();
    params.registerBase("Kernel");
    params.addParam<std::string>("variable", "Variable on which to apply the kernel");
    return params;
  };

  MFEMKernel(const InputParameters & parameters) : MFEMGeneralUserObject(parameters) {}
  virtual ~MFEMKernel() = default;

  virtual T * createIntegrator() = 0;
};
