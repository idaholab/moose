//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMScalarFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMScalarFunctorDirichletBC);

InputParameters
MFEMScalarFunctorDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient",
      "The coefficient setting the values on the essential boundary. A coefficient can be any of "
      "the "
      "following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

MFEMScalarFunctorDirichletBC::MFEMScalarFunctorDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarCoefficient(_coef_name))
{
}

void
MFEMScalarFunctorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(_coef, ess_bdrs);
}

#endif
