//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarDirichletBC.h"

registerMooseMFEMObject("MooseApp", ScalarDirichletBC);

namespace Moose::MFEM
{
InputParameters
ScalarDirichletBC::validParams()
{
  InputParameters params = EssentialBC::validParams();
  params.addClassDescription("Applies a Dirichlet condition to a scalar variable.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "0.", "The coefficient setting the values on the essential boundary");
  return params;
}

ScalarDirichletBC::ScalarDirichletBC(const InputParameters & parameters)
  : EssentialBC(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
ScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_coef, getBoundaryMarkers());
}

} // namespace Moose::MFEM
#endif
