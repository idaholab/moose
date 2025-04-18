#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"

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
    params.addParam<VariableName>("variable",
                                  "Variable labelling the weak form this kernel is added to");
    return params;
  }

  MFEMKernel(const InputParameters & parameters)
    : MFEMGeneralUserObject(parameters), _test_var_name(getParam<VariableName>("variable"))
  {
  }
  virtual ~MFEMKernel() = default;

  // Create a new MFEM integrator to apply to the weak form. Ownership managed by the caller.
  virtual T * createIntegrator() = 0;

  // Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const { return _test_var_name; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName _test_var_name;
};

#endif
