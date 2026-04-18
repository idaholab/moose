//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDomainSubMesh.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", DomainSubMesh);

namespace Moose::MFEM
{
InputParameters
DomainSubMesh::validParams()
{
  InputParameters params = SubMesh::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Class to construct an Moose::MFEM::SubMesh formed from the subspace of the "
      "parent mesh restricted to the set of user-specified subdomains.");
  params.addParam<BoundaryName>("submesh_boundary", "Name to assign submesh boundary.");
  return params;
}

DomainSubMesh::DomainSubMesh(const InputParameters & parameters)
  : SubMesh(parameters), BlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh())
{
}

void
DomainSubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(
      mfem::ParSubMesh::CreateFromDomain(getMesh(), getSubdomainAttributes()));
  _submesh->attribute_sets.attr_sets = getMesh().attribute_sets.attr_sets;
  _submesh->bdr_attribute_sets.attr_sets = getMesh().bdr_attribute_sets.attr_sets;

  if (isParamSetByUser("submesh_boundary"))
    _submesh->bdr_attribute_sets.SetAttributeSet(
        getParam<BoundaryName>("submesh_boundary"),
        mfem::Array<int>({getMesh().bdr_attributes.Max() + 1}));
}

} // namespace Moose::MFEM
#endif
