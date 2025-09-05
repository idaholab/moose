//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorDirichletBC.h"

registerMooseObject("MooseApp", MFEMComplexVectorDirichletBC);

InputParameters
MFEMComplexVectorDirichletBC::validParams()
{
  InputParameters params = MFEMComplexVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a complex Dirichlet condition to all components of a vector variable.");
  return params;
}

MFEMComplexVectorDirichletBC::MFEMComplexVectorDirichletBC(const InputParameters & parameters)
  : MFEMComplexVectorDirichletBCBase(parameters)
{
}

void
MFEMComplexVectorDirichletBC::ApplyBC(mfem::ParComplexGridFunction & /*gridfunc*/)
{
  mooseError("Full complex Dirichlet BCs not implemented for vector variables. Please use "
             "Tangential or Normal complex Dirichlet BCs instead.");
}

#endif
