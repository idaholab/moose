#include "MFEMVariable.h"
#include "MooseVariableBase.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("PlatypusApp", MFEMVariable);

InputParameters
MFEMVariable::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries.
  params.addRequiredParam<UserObjectName>("fespace",
                                          "The finite element space this variable is defined on.");
  params.addParam<FunctionName>("initial_value",
                                "",
                                "The value with which to initialise this variable. If left blank "
                                "then the variable is initialised to 0.");
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
    _gridfunction(buildGridFunction()),
    _initial(getParam<FunctionName>("initial_value"))
{
  *_gridfunction = 0.0;
  initialise();
}

void
MFEMVariable::initialise()
{
  if (_initial == "")
  {
    return;
  }
  if (getMFEMProblem().hasScalarFunctionCoefficient(_initial))
  {
    _gridfunction->ProjectCoefficient(*getMFEMProblem().getScalarFunctionCoefficient(_initial));
  }
  else if (getMFEMProblem().hasVectorFunctionCoefficient(_initial))
  {
    _gridfunction->ProjectCoefficient(*getMFEMProblem().getVectorFunctionCoefficient(_initial));
  }
  else
  {
    mooseError("Unrecognised function name '" + _initial + "'");
  }
}

const std::shared_ptr<mfem::ParGridFunction>
MFEMVariable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_fespace.getFESpace().get());
}
