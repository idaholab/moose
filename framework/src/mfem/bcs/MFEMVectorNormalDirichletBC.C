//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorNormalDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorNormalDirichletBC);

InputParameters
MFEMVectorNormalDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the normal components of a vector variable.");
  return params;
}

MFEMVectorNormalDirichletBC::MFEMVectorNormalDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorNormalDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientNormal(_vec_coef, getBoundaryMarkers());
}

#endif
