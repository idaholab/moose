//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMScalarDirichletBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarDirichletBC);

InputParameters
MFEMScalarDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addRequiredParam<Real>("value", "The scalar value to use in the Dirichlet condition");
  return params;
}

MFEMScalarDirichletBC::MFEMScalarDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef(getMFEMProblem().getCoefficients().declareScalar<mfem::ConstantCoefficient>(
        "__ScalarDirichletBC_" + parameters.get<std::string>("_unique_name"),
        getParam<Real>("value")))
{
}

void
MFEMScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef, getBoundaries());
}

#endif
