//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMDomainSubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainSubMesh);

InputParameters
MFEMDomainSubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params.addClassDescription("Class to construct an MFEMSubMesh formed from the subspace of the "
                             "parent mesh restricted to the set of user-specified subdomains.");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "block", "The list of subdomain blocks ids comprising this SubMesh");
  return params;
}

MFEMDomainSubMesh::MFEMDomainSubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  for (const auto i : index_range(_subdomain_names))
    _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
}

void
MFEMDomainSubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromDomain(
      getMFEMProblem().mesh().getMFEMParMesh(), getSubdomains()));
}

#endif
