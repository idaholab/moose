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

registerMooseObject("MooseApp", MFEMScalarProjectionAux);

InputParameters
MFEMScalarProjectionAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects a scalar coefficient onto a scalar MFEMVariable");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient",
                                                     "Name of the scalar coefficient to project.");
  return params;
}

MFEMScalarProjectionAux::MFEMScalarProjectionAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
MFEMScalarProjectionAux::execute()
{
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_coef);
}

#endif
