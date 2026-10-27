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
  params.addClassDescription("Projects a vector coefficient $\\vec u$, optionally scaled by a "
                             "scalar coefficient $k$, onto a vector MFEM auxvariable, storing "
                             "$k \\vec u$.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient",
                                                     "Name of the vector coefficient to project.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of the scalar coefficient k to scale the vector coefficient by.");
  return params;
}

MFEMVectorProjectionAux::MFEMVectorProjectionAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _vec_coef(getScalarCoefficient("coefficient"), getVectorCoefficient("vector_coefficient"))
{
}

void
MFEMVectorProjectionAux::execute()
{
  _result_var.ProjectCoefficient(_vec_coef);
}

#endif
