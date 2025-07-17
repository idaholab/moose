#ifdef MFEM_ENABLED

#include "MFEMIndicator.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMIndicator);

// static method
InputParameters
MFEMIndicator::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Indicator");

  params.addRequiredParam<std::string>("variable", "Variable to perform amr with");
  params.addRequiredParam<std::string>("kernel", "Kernel to perform amr with");

  return params;
}

MFEMIndicator::MFEMIndicator(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _variable_name(getParam<std::string>("variable")),
    _variable(getUserObject<MFEMVariable>("variable")),
    _kernel_name(getParam<std::string>("kernel"))
{
}

std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMIndicator::getFESpace() const
{
  // MFEMVariable::getFESpace() returns a reference to the MFEMFESpace
  // and we piggyback from this to get the underlying shared ptr
  return _variable.getFESpace().getFESpace();
}

#endif
