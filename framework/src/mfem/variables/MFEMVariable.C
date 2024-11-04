#include "MFEMVariable.h"
#include "MooseVariableBase.h"
#include "mfem.hpp"

registerMooseObject("PlatypusApp", MFEMVariable);

InputParameters
MFEMVariable::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries.
  params.addRequiredParam<UserObjectName>("fespace",
                                          "The finite element space this variable is defined on.");
  // Require moose variable parameters (not used!)
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding MFEM variables to the problem (`mfem::ParGridFunction`s).");
  params.registerBase("MooseVariableBase");
  return params;
}

MFEMVariable::MFEMVariable(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _fespace(getUserObject<MFEMFESpace>("fespace")),
    _gridfunction(buildGridFunction())
{
  *_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParGridFunction>
MFEMVariable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_fespace.getFESpace().get());
}
