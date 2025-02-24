#include "MFEMVectorFESpace.h"

InputParameters
MFEMVectorFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSimplifiedFESpace");
  params.addClassDescription(
      "Convenience class to construct vector finite element spaces, abstracting away some of the "
      "mathematical complexity of specifying the dimensions.");
  MooseEnum fec_types("H1 ND RT L2", "H1", true);
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  params.addParam<int>("range_dim",
                       0,
                       "The number of components of the vectors in reference space. Note that MFEM "
                       "does not currently support 2D "
                       "vectors in 1D space for ND and RT elements.");
  return params;
}

MFEMVectorFESpace::MFEMVectorFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters),
    _fec_type(parameters.get<MooseEnum>("fec_type")),
    _range_dim(parameters.get<int>("range_dim"))
{
}

std::string
MFEMVectorFESpace::getFECName() const
{
  const int pdim = getProblemDim();
  std::string dim_qualifier = "";
  if ((_fec_type == "ND" || _fec_type == "RT") && _range_dim != pdim)
  {
    if (_range_dim != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_range_dim) + "D vectors in " + std::to_string(pdim) + "D space.");
    dim_qualifier = "R";
  }

  return _fec_type + "_" + dim_qualifier + std::to_string(pdim) + "D_P" +
         std::to_string(_fec_order);
}

int
MFEMVectorFESpace::getVDim() const
{
  if (_fec_type == "H1" || _fec_type == "L2")
  {
    return _range_dim;
  }
  else
  {
    return 1;
  }
}
