//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSubMesh.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
InputParameters
SubMesh::validParams()
{
  InputParameters params = Object::validParams();
  params.registerBase("Moose::MFEM::SubMesh");
  params.registerSystemAttributeName("Moose::MFEM::SubMesh");
  return params;
}

SubMesh::SubMesh(const InputParameters & parameters) : Object(parameters) {}

} // namespace Moose::MFEM
#endif
