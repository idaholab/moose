//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScaledVectorAux.h"

registerMooseObject("MooseApp", MFEMScaledVectorAux);

InputParameters
MFEMScaledVectorAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects the product of a scalar coefficient and a vector "
                             "coefficient, $k \\vec u$, onto a vector MFEM auxvariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient",
                                                     "Name of the vector coefficient to scale.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of the scalar coefficient k to scale it by.");
  return params;
}

MFEMScaledVectorAux::MFEMScaledVectorAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _scaled_vec_coef(getScalarCoefficient("coefficient"),
                     getVectorCoefficient("vector_coefficient"))
{
}

void
MFEMScaledVectorAux::execute()
{
  _result_var.ProjectCoefficient(_scaled_vec_coef);
}

#endif
