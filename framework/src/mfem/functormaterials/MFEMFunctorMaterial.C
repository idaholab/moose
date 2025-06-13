//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMFunctorMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMFunctorMaterial);

InputParameters
MFEMFunctorMaterial::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription(
      "Base class for declaration of material properties to add to MFEM problems.");
  params.set<std::string>("_moose_base") = "FunctorMaterial";
  params.addPrivateParam<bool>("_neighbor", false);
  params.addPrivateParam<bool>("_interface", false);

  params.addParam<std::vector<SubdomainName>>(
      "block",
      {},
      "The list of blocks (ids or names) that this object will be applied to. Leave empty to apply "
      "to all blocks.");
  return params;
}

std::vector<std::string>
MFEMFunctorMaterial::subdomainsToStrings(const std::vector<SubdomainName> & blocks)
{
  std::vector<std::string> result(blocks.size());
  // FIXME: Is there really no better way to do this conversion? It doesn't seem like it should be
  // necessary to do the various copies etc. for this.
  std::transform(blocks.begin(),
                 blocks.end(),
                 result.begin(),
                 // FIXME: How do I pass the string constructor directly?
                 [](const SubdomainName & x) -> std::string { return std::string(x); });
  return result;
}

libMesh::Point
MFEMFunctorMaterial::pointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(vec.Elem(0), vec.Elem(1), vec.Elem(2));
}

MFEMFunctorMaterial::MFEMFunctorMaterial(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _block_ids(getParam<std::vector<SubdomainName>>("block")),
    _properties(getMFEMProblem().getCoefficients())
{
}

MFEMFunctorMaterial::~MFEMFunctorMaterial() {}

#endif
