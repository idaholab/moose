//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorNormalDirichletBC.h"

registerMooseMFEMObject("MooseApp", ComplexVectorNormalDirichletBC);

namespace Moose::MFEM
{
InputParameters
ComplexVectorNormalDirichletBC::validParams()
{
  InputParameters params = ComplexVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a complex Dirichlet condition to the normal components of a vector variable.");
  return params;
}

ComplexVectorNormalDirichletBC::ComplexVectorNormalDirichletBC(const InputParameters & parameters)
  : ComplexVectorDirichletBCBase(parameters)
{
}

void
ComplexVectorNormalDirichletBC::ApplyBC(mfem::ParComplexGridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientNormal(_vec_coef_real, _vec_coef_imag, getBoundaryMarkers());
}

} // namespace Moose::MFEM
#endif
