//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexVectorDirichletBCBase.h"

namespace Moose::MFEM
{
InputParameters
ComplexVectorDirichletBCBase::validParams()
{
  InputParameters params = ComplexEssentialBC::validParams();
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient_real",
      "0. 0. 0.",
      "Vector coefficient specifying the real part of the "
      "values the variable takes on the boundary");
  params.addParam<Moose::MFEM::VectorCoefficientName>(
      "vector_coefficient_imag",
      "0. 0. 0.",
      "Vector coefficient specifying the imaginary part of "
      "the values the variable takes on the boundary");

  return params;
}

ComplexVectorDirichletBCBase::ComplexVectorDirichletBCBase(const InputParameters & parameters)
  : ComplexEssentialBC(parameters),
    _vec_coef_real(getVectorCoefficient("vector_coefficient_real")),
    _vec_coef_imag(getVectorCoefficient("vector_coefficient_imag"))
{
}

} // namespace Moose::MFEM
#endif
