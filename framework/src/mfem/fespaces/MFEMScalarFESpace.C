#include "MFEMScalarFESpace.h"

registerMooseObject("MooseApp", MFEMScalarFESpace);

InputParameters
MFEMScalarFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription("Convenience class to construct scalar finite element spaces.");
  MooseEnum fec_types("H1 L2", "H1", true);
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  return params;
}

MFEMScalarFESpace::MFEMScalarFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters), _fec_type(parameters.get<MooseEnum>("fec_type"))
{
}

std::string
MFEMScalarFESpace::getFECName() const
{
  return _fec_type + "_" + std::to_string(getProblemDim()) + "D_P" + std::to_string(_fec_order);
}

int
MFEMScalarFESpace::getVDim() const
{
  return 1;
}
