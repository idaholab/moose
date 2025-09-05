//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMComplexVectorTangentialDirichletBC);

InputParameters
MFEMComplexVectorTangentialDirichletBC::validParams()
{
  InputParameters params = MFEMComplexVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a complex Dirichlet condition to the tangential components of a vector variable.");
  return params;
}

MFEMComplexVectorTangentialDirichletBC::MFEMComplexVectorTangentialDirichletBC(
    const InputParameters & parameters)
  : MFEMComplexVectorDirichletBCBase(parameters)
{
}

void
MFEMComplexVectorTangentialDirichletBC::ApplyBC(mfem::ParComplexGridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientTangent(_vec_coef_real, _vec_coef_imag, getBoundaryMarkers());
}

#endif
