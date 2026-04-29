//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEML2Error.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", L2Error);

namespace Moose::MFEM
{
InputParameters
L2Error::validParams()
{
  InputParameters params = Postprocessor::validParams();
  params.addClassDescription(
      "Computes L2 error $\\left\\Vert u_{ex} - u_{h}\\right\\Vert_{\\rm L2}$ for "
      "gridfunctions using H1 or L2 elements.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>("function",
                                                      "The analytic solution to compare against.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "Name of the variable of which to find the norm of the error.");
  return params;
}

L2Error::L2Error(const InputParameters & parameters)
  : Postprocessor(parameters),
    _coeff(getScalarCoefficient("function")),
    _var(*getMFEMProblem().getGridFunction(getParam<VariableName>("variable")))
{
}

void
L2Error::initialize()
{
}

void
L2Error::execute()
{
}

PostprocessorValue
L2Error::getValue() const
{
  return _var.ComputeL2Error(_coeff);
}

} // namespace Moose::MFEM
#endif
