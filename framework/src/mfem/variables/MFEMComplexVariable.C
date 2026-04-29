#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVariable.h"
#include "MooseVariableBase.h"
#include "MFEMProblem.h"
#include "MFEMFESpace.h"

registerMooseMFEMObject("MooseApp", ComplexVariable);

namespace Moose::MFEM
{
InputParameters
ComplexVariable::validParams()
{
  InputParameters params = Object::validParams();
  params.addRequiredParam<Moose::MFEM::FESpaceName>(
      "fespace", "The finite element space this variable is defined on.");
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding complex MFEM variables to the problem (`mfem::ParComplexGridFunction`s).");
  params.registerBase("MooseVariableBase");
  params.registerSystemAttributeName("MooseVariableBase");

  return params;
}

ComplexVariable::ComplexVariable(const InputParameters & parameters)
  : Object(parameters),
    _fespace(getMFEMProblem().getMFEMObject<FESpace>(
        "Moose::MFEM::FESpace", getParam<Moose::MFEM::FESpaceName>("fespace"))),
    _cmplx_gridfunction(buildComplexGridFunction())
{
  *_cmplx_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParComplexGridFunction>
ComplexVariable::buildComplexGridFunction()
{
  return std::make_shared<mfem::ParComplexGridFunction>(_fespace.getFESpace().get());
}

} // namespace Moose::MFEM
#endif
