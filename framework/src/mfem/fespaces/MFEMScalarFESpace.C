//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarFESpace.h"

registerMooseObject("MooseApp", MFEMScalarFESpace);

InputParameters
MFEMScalarFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription("Convenience class to construct scalar finite element spaces.");
  MooseEnum fec_types("H1 L2", "H1");
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  MooseEnum basis_types("GaussLegendre GaussLobatto Positive OpenUniform ClosedUniform "
                        "OpenHalfUniform Serendipity ClosedGL IntegratedGLL",
                        "GaussLobatto");
  params.addParam<MooseEnum>(
      "basis",
      basis_types,
      "Specifies the basis used for scalar elements. H1 spaces require a closed basis "
      "(GaussLobatto Positive ClosedUniform Serendipity ClosedGL)");
  MooseEnum fec_maps("VALUE INTEGRAL","VALUE",true);
  params.addParam<MooseEnum>("fec_map", fec_maps, "Specify the FE map type used VALUE or INTEGRAL (meaningfull for L2 only)");

  return params;
}

MFEMScalarFESpace::MFEMScalarFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters), _fec_type(getParam<MooseEnum>("fec_type")),
  _fec_map(getParam<MooseEnum>("fec_map"))
{
}

std::string
MFEMScalarFESpace::getFECName() const
{
  std::string basis =
      _fec_type == "L2"
          ? "_T" + std::to_string(getParam<MooseEnum>("basis"))
          : "@" + std::string({mfem::BasisType::GetChar(getParam<MooseEnum>("basis"))});

  // This is to get around an MFEM bug (to be removed in #31525)
  basis = (basis == "@G" || basis == "_T0") ? "" : basis;

  if (_fec_map == "INTEGRAL")
    return "L2Int" + basis + "_" + std::to_string(getProblemDim()) + "D_P" +
    std::to_string(_fec_order);

  return _fec_type + basis + "_" + std::to_string(getProblemDim()) + "D_P" +
         std::to_string(_fec_order);
}

int
MFEMScalarFESpace::getVDim() const
{
  return 1;
}

#endif
