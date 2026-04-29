//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorTangentialDirichletBC.h"

registerMooseMFEMObject("MooseApp", VectorTangentialDirichletBC);

namespace Moose::MFEM
{
InputParameters
VectorTangentialDirichletBC::validParams()
{
  InputParameters params = VectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the tangential components of a vector variable.");
  return params;
}

VectorTangentialDirichletBC::VectorTangentialDirichletBC(const InputParameters & parameters)
  : VectorDirichletBCBase(parameters)
{
}

void
VectorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientTangent(_vec_coef, getBoundaryMarkers());
}

} // namespace Moose::MFEM
#endif
