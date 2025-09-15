//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFESpace.h"

registerMooseObject("MooseApp", MFEMVectorFESpace);

InputParameters
MFEMVectorFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription(
      "Convenience class to construct vector finite element spaces, abstracting away some of the "
      "mathematical complexity of specifying the dimensions.");
  MooseEnum fec_types("H1 ND RT L2 L2Int", "H1");
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  MooseEnum closed_basis_types("GaussLobatto=1 ClosedUniform=4 Serendipity=6 ClosedGL=7",
                               "GaussLobatto");
  params.addParam<MooseEnum>("closed_basis",
                             closed_basis_types,
                             "Specifies the closed basis used for ND and RT elements.");
  MooseEnum basis_types("GaussLegendre GaussLobatto Positive OpenUniform ClosedUniform "
                        "OpenHalfUniform Serendipity ClosedGL IntegratedGLL",
                        "GaussLegendre");
  params.addParam<MooseEnum>(
      "open_basis", basis_types, "Specifies the open basis used for ND and RT elements.");
  basis_types = "GaussLobatto";
  params.addParam<MooseEnum>(
      "basis",
      basis_types,
      "Specifies the basis used for H1 and L2(Int) vector elements. H1 spaces require a closed "
      "basis (GaussLobatto Positive ClosedUniform Serendipity ClosedGL)");
  params.addParam<int>("range_dim",
                       0,
                       "The number of components of the vectors in reference space. Zero "
                       "(the default) means it will be the same as the problem dimension. "
                       "Note that MFEM does not currently support 2D vectors in 1D space "
                       "for ND and RT elements.");

  return params;
}

MFEMVectorFESpace::MFEMVectorFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters),
    _fec_type(getParam<MooseEnum>("fec_type")),
    _range_dim(getParam<int>("range_dim"))
{
}

std::string
MFEMVectorFESpace::getFECName() const
{
  const int pdim = getProblemDim();
  std::string actual_type = _fec_type;
  if ((_fec_type == "ND" || _fec_type == "RT") && _range_dim != 0 && _range_dim != pdim)
  {
    if (_range_dim != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_range_dim) + "D vectors in " + std::to_string(pdim) + "D space.");
    actual_type += "_R" + std::to_string(pdim) + "D";
  }

  std::string basis = _fec_type == "L2" || _fec_type == "L2Int" ? "_T" : "@";

  if (_fec_type == "ND" || _fec_type == "RT")
  {
    if (isParamSetByUser("basis"))
      mooseWarning("basis parameter ignored, using closed_basis/open_basis parameters instead");
    basis += mfem::BasisType::GetChar(getParam<MooseEnum>("closed_basis"));
    basis += mfem::BasisType::GetChar(getParam<MooseEnum>("open_basis"));
  }
  else if (_fec_type == "H1" || _fec_type == "L2" || _fec_type == "L2Int")
  {
    if (isParamSetByUser("closed_basis") || isParamSetByUser("open_basis"))
      mooseWarning("closed_basis/open_basis parameter ignored, using basis parameter instead");
    basis += _fec_type == "L2" || _fec_type == "L2Int"
                 ? std::to_string(getParam<MooseEnum>("basis"))
                 : std::string({mfem::BasisType::GetChar(getParam<MooseEnum>("basis"))});
  }

  return actual_type + basis + "_" + std::to_string(pdim) + "D_P" + std::to_string(_fec_order);
}

int
MFEMVectorFESpace::getVDim() const
{
  if (_fec_type == "H1" || _fec_type == "L2" || _fec_type == "L2Int")
  {
    return _range_dim == 0 ? getProblemDim() : _range_dim;
  }
  else
  {
    return 1;
  }
}

#endif
