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

registerMooseMFEMObject("MooseApp", ScalarFESpace);

namespace Moose::MFEM
{
InputParameters
ScalarFESpace::validParams()
{
  InputParameters params = SimplifiedFESpace::validParams();
  params.addClassDescription("Convenience class to construct scalar finite element spaces.");
  MooseEnum fec_types("H1 L2 L2Int", "H1");
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  MooseEnum basis_types("GaussLegendre GaussLobatto Positive OpenUniform ClosedUniform "
                        "OpenHalfUniform Serendipity ClosedGL IntegratedGLL",
                        "GaussLobatto");
  params.addParam<MooseEnum>(
      "basis",
      basis_types,
      "Specifies the basis used for scalar elements. H1 spaces require a closed basis "
      "(GaussLobatto Positive ClosedUniform Serendipity ClosedGL)");

  return params;
}

ScalarFESpace::ScalarFESpace(const InputParameters & parameters)
  : SimplifiedFESpace(parameters), _fec_type(getParam<MooseEnum>("fec_type"))
{
}

std::string
ScalarFESpace::getFECName() const
{
  std::string basis =
      _fec_type == "L2" || _fec_type == "L2Int"
          ? "_T" + std::to_string(getParam<MooseEnum>("basis"))
          : "@" + std::string({mfem::BasisType::GetChar(getParam<MooseEnum>("basis"))});

  return _fec_type + basis + "_" + std::to_string(getProblemDim()) + "D_P" +
         std::to_string(_fec_order);
}

int
ScalarFESpace::getVDim() const
{
  return 1;
}

} // namespace Moose::MFEM
#endif
