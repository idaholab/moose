//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorL2Error.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", VectorL2Error);

namespace Moose::MFEM
{
InputParameters
VectorL2Error::validParams()
{
  InputParameters params = Postprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert \\vec u_{ex} - \\vec u_{h}\\right\\Vert_{\\rm L2}$ for "
      "vector gridfunctions.");
  params.addParam<Moose::MFEM::VectorCoefficientName>("function",
                                                      "The analytic solution to compare against.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the vector variable of which to find the norm of the error.");
  return params;
}

VectorL2Error::VectorL2Error(const InputParameters & parameters)
  : Postprocessor(parameters),
    _vec_coeff(getVectorCoefficient("function")),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable")))
{
}

void
VectorL2Error::initialize()
{
}

void
VectorL2Error::execute()
{
}

PostprocessorValue
VectorL2Error::getValue() const
{
  return _var.ComputeL2Error(_vec_coeff);
}

} // namespace Moose::MFEM
#endif
