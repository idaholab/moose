//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMScalarFESpace.h"

registerMooseObject("MooseApp", MFEMScalarFESpace);

InputParameters
MFEMScalarFESpace::validParams()
{
  InputParameters params = MFEMSimplifiedFESpace::validParams();
  params.addClassDescription("Convenience class to construct scalar finite element spaces.");
  MooseEnum fec_types("H1 L2", "H1", true);
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  params.addParam<std::string>(
      "basis", "GaussLobatto", "Specifies the quadrature basis used for scalar elements.");

  return params;
}

MFEMScalarFESpace::MFEMScalarFESpace(const InputParameters & parameters)
  : MFEMSimplifiedFESpace(parameters), _fec_type(parameters.get<MooseEnum>("fec_type"))
{
}

std::string
MFEMScalarFESpace::getFECName() const
{
  char b = mfem::BasisType::GetChar(getBasis(getParam<std::string>("basis")));
  std::string basis(1, b);

  if (_fec_type == "H1")
    basis = (basis == "G" ? "" : "@" + basis);
  else if (_fec_type == "L2")
  {
    if (basis != "g")
      mooseInfo("L2 finite element space only supports GaussLegendre basis. "
                "Ignoring " +
                getParam<std::string>("basis") +
                " basis choice and using GaussLegendre instead.\n");
    basis = "";
  }

  return _fec_type + basis + "_" + std::to_string(getProblemDim()) + "D_P" +
         std::to_string(_fec_order);
}

int
MFEMScalarFESpace::getVDim() const
{
  return 1;
}

#endif
