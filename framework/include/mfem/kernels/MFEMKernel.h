#pragma once

#include "MFEMGeneralUserObject.h"
#include "gridfunctions.h"

/*
Class to construct an MFEM integrator to apply to the equation system.

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

  MFEMKernel(const InputParameters & parameters)
    : MFEMGeneralUserObject(parameters), _test_var_name(getParam<std::string>("variable"))
  {
  }
  virtual ~MFEMKernel() = default;

  // Create a new MFEM integrator to apply to the weak form. Ownership managed by the caller.
  virtual T * createIntegrator() = 0;

protected:
  // Name of the (test) variable associated with the weak form kernel is applied to.
  std::string _test_var_name;
};
