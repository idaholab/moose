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

registerMooseMFEMObject("MooseApp", VectorProjectionAux);

namespace Moose::MFEM
{
InputParameters
VectorProjectionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Projects a vector coefficient onto a vector MFEM auxvariable.");
  params.addRequiredParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient", "Name of the vector coefficient to project.");
  return params;
}

VectorProjectionAux::VectorProjectionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

void
VectorProjectionAux::execute()
{
  _result_var.ProjectCoefficient(_vec_coef);
}

} // namespace Moose::MFEM
#endif
