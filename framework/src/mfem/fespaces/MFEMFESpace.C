//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMFESpace.h"
#include "MFEMProblem.h"

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params.addParam<int>("vdim", 1, "The number of degrees of freedom per basis function.");
  params.addParam<std::string>("submesh",
                               "Submesh to define the FESpace on. Leave blank to use base mesh.");
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _ordering(parameters.get<MooseEnum>("ordering")),
    _pmesh(
        parameters.isParamValid("submesh")
            ? getMFEMProblem().getProblemData().submeshes.GetRef(getParam<std::string>("submesh"))
            : const_cast<mfem::ParMesh &>(getMFEMProblem().mesh().getMFEMParMesh()))
{
}

void
MFEMFESpace::buildFEC(const std::string & fec_name) const
{
  auto name = fec_name.c_str();
  // Handle a few cases that were not supported by mfem::FiniteElementCollection::New as of v4.7
  if (!strncmp(name, "RT_R1D", 6))
  {
    _fec = std::make_shared<mfem::RT_R1D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "RT_R2D", 6))
  {
    _fec = std::make_shared<mfem::RT_R2D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "ND_R1D", 6))
  {
    _fec = std::make_shared<mfem::ND_R1D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "ND_R2D", 6))
  {
    _fec = std::make_shared<mfem::ND_R2D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else
  {
    _fec = std::shared_ptr<mfem::FiniteElementCollection>(mfem::FiniteElementCollection::New(name));
  }
}

int
MFEMFESpace::getBasis(const std::string & basis_name) const
{

  if (!strcmp(basis_name.c_str(), "GaussLegendre"))
    return mfem::BasisType::GaussLegendre;
  else if (!strcmp(basis_name.c_str(), "GaussLobatto"))
    return mfem::BasisType::GaussLobatto;
  else if (!strcmp(basis_name.c_str(), "Positive"))
    return mfem::BasisType::Positive;
  else if (!strcmp(basis_name.c_str(), "OpenUniform"))
    return mfem::BasisType::OpenUniform;
  else if (!strcmp(basis_name.c_str(), "ClosedUniform"))
    return mfem::BasisType::ClosedUniform;
  else if (!strcmp(basis_name.c_str(), "OpenHalfUniform"))
    return mfem::BasisType::OpenHalfUniform;
  else if (!strcmp(basis_name.c_str(), "Serendipity"))
    return mfem::BasisType::Serendipity;
  else if (!strcmp(basis_name.c_str(), "ClosedGL"))
    return mfem::BasisType::ClosedGL;
  else if (!strcmp(basis_name.c_str(), "IntegratedGLL"))
    return mfem::BasisType::IntegratedGLL;
  else if (!strcmp(basis_name.c_str(), "NumBasisTypes"))
    return mfem::BasisType::NumBasisTypes;
  else
    mooseError("Unknown basis type: ", basis_name);
}

void
MFEMFESpace::buildFESpace(const int vdim) const
{
  _fespace =
      std::make_shared<mfem::ParFiniteElementSpace>(&_pmesh, getFEC().get(), vdim, _ordering);
}

#endif
