#include "NodalPatchRecoveryVariable.h"

registerMooseObject("MooseApp", NodalPatchRecoveryVariable);

InputParameters
NodalPatchRecoveryVariable::validParams()
{
  InputParameters params = NodalPatchRecoveryBase::validParams();

  params.addRequiredCoupledVar("var", "The scalar variable to recover at nodes");

  // params.addParam<unsigned int>("component", 0, "Component index (if variable has multiple)");

  return params;
}

NodalPatchRecoveryVariable::NodalPatchRecoveryVariable(const InputParameters & params)
  : NodalPatchRecoveryBase(params), _var(coupledValue("var"))
{
}
