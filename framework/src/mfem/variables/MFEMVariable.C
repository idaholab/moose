#include "MFEMVariable.h"

registerMooseObject("PlatypusApp", MFEMVariable);

InputParameters
MFEMVariable::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.registerBase("MooseVariableBase");

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries

  params.addRequiredParam<UserObjectName>("fespace",
                                          "The finite element space this variable is defined on.");

  // Remaining params are for compatibility with MOOSE AuxVariable interface
  params.addRangeCheckedParam<unsigned int>(
      "components", 3, "components>0", "Number of components for an array variable");

  params.addParam<std::vector<Real>>("scaling",
                                     "Specifies a scaling factor to apply to this variable");
  params.addParam<bool>("eigen", false, "True to make this variable an eigen variable");
  params.addParam<bool>("fv", false, "True to make this variable a finite volume variable");
  params.addParam<bool>("array",
                        false,
                        "True to make this variable a array variable regardless of number of "
                        "components. If 'components' > 1, this will automatically be set to"
                        "true.");
  params.addParamNamesToGroup("scaling eigen", "Advanced");

  params.addParam<bool>("use_dual", false, "True to use dual basis for Lagrange multipliers");

  return params;
}

MFEMVariable::MFEMVariable(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    fespace(getUserObject<MFEMFESpace>("fespace")),
    components(parameters.get<unsigned int>("components"))
{
}

MFEMVariable::~MFEMVariable() {}
