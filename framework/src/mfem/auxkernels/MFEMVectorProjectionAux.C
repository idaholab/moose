//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorProjectionAux.h"

registerMooseObject("MooseApp", MFEMVectorProjectionAux);

InputParameters
MFEMVectorProjectionAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects a vector coefficient onto a vector MFEM auxvariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient",
                                                     "Name of the vector coefficient to project.");
  return params;
}

MFEMVectorProjectionAux::MFEMVectorProjectionAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

void
MFEMVectorProjectionAux::execute()
{
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_vec_coef);
}

#endif
