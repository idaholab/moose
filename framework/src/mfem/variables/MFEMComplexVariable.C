#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVariable.h"
#include "MooseVariableBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

registerMooseObject("MooseApp", MFEMComplexVariable);

InputParameters
MFEMComplexVariable::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>("fespace",
                                          "The finite element space this variable is defined on.");
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding complex MFEM variables to the problem (`mfem::ParComplexGridFunction`s).");
  params.registerBase("MooseVariableBase");

  return params;
}

MFEMComplexVariable::MFEMComplexVariable(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _fespace(getUserObject<MFEMFESpace>("fespace")),
    _cmplx_gridfunction(buildComplexGridFunction())
{
  *_cmplx_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParComplexGridFunction>
MFEMComplexVariable::buildComplexGridFunction()
{
  return std::make_shared<mfem::ParComplexGridFunction>(_fespace.getFESpace().get());
}

#endif
