#include "MFEMVariable.h"
#include "mfem.hpp"

registerMooseObject("PlatypusApp", MFEMVariable);

InputParameters
MFEMVariable::validParams()
{
  InputParameters params = MooseVariableBase::validParams();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries.
  params.addRequiredParam<UserObjectName>("fespace",
                                          "The finite element space this variable is defined on.");

  // Set moose required parameters with dummy options since we never actually use them.
  params.set<MooseEnum>("order") = "CONSTANT";
  params.set<MooseEnum>("family") = "SCALAR";

  params.addClassDescription("Class for MFEM variables (gridfunctions).");

  return params;
}

MFEMVariable::MFEMVariable(const InputParameters & parameters)
  : MooseVariableBase(parameters),
    _fespace(parameters.get<MFEMFESpace>("fespace")),
    _gridfunction(buildGridFunction())
{
}

const std::shared_ptr<mfem::ParGridFunction>
MFEMVariable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_fespace.getFESpace().get());
}
