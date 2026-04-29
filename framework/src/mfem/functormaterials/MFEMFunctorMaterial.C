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

namespace Moose::MFEM
{
InputParameters
FunctorMaterial::validParams()
{
  InputParameters params = Object::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();

  params.addClassDescription(
      "Base class for declaration of material properties to add to MFEM problems.");
  params.registerBase("FunctorMaterial");
  params.registerSystemAttributeName("FunctorMaterial");
  params.addPrivateParam<bool>("_neighbor", false);
  params.addPrivateParam<bool>("_interface", false);
  return params;
}

libMesh::Point
FunctorMaterial::pointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(vec.Elem(0), vec.Elem(1), vec.Elem(2));
}

FunctorMaterial::FunctorMaterial(const InputParameters & parameters)
  : Object(parameters),
    BlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    BoundaryRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
    _properties(getMFEMProblem().getCoefficients())
{
  if (isSubdomainRestricted() && isBoundaryRestricted())
    paramError("boundary", "Cannot specify both block and boundary parameters");
}

FunctorMaterial::~FunctorMaterial() {}

} // namespace Moose::MFEM
#endif
