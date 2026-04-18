//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMFunctorMaterial.h"
#include "MFEMProblem.h"

InputParameters
MFEMFunctorMaterial::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params += MFEMBlockRestrictable::validParams();
  params += MFEMBoundaryRestrictable::validParams();

  params.addClassDescription(
      "Base class for declaration of material properties to add to MFEM problems.");
  params.registerBase("FunctorMaterial");
  params.registerSystemAttributeName("FunctorMaterial");
  params.addPrivateParam<bool>("_neighbor", false);
  params.addPrivateParam<bool>("_interface", false);
  return params;
}

libMesh::Point
MFEMFunctorMaterial::pointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(vec.Elem(0), vec.Elem(1), vec.Elem(2));
}

MFEMFunctorMaterial::MFEMFunctorMaterial(const InputParameters & parameters)
  : MFEMObject(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    MFEMBoundaryRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _properties(getMFEMProblem().getCoefficients())
{
  if (isSubdomainRestricted() && isBoundaryRestricted())
    paramError("boundary", "Cannot specify both block and boundary parameters");
}

MFEMFunctorMaterial::~MFEMFunctorMaterial() {}

#endif
