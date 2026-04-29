//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarProjectionAux.h"

registerMooseMFEMObject("MooseApp", ScalarProjectionAux);

namespace Moose::MFEM
{
InputParameters
ScalarProjectionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Projects a scalar coefficient onto a scalar MFEM auxvariable");
  params.addRequiredParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "Name of the scalar coefficient to project.");
  return params;
}

ScalarProjectionAux::ScalarProjectionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
ScalarProjectionAux::execute()
{
  _result_var.ProjectCoefficient(_coef);
}

} // namespace Moose::MFEM
#endif
