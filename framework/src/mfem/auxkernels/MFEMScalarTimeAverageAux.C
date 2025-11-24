//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarTimeAverageAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarTimeAverageAux);

InputParameters
MFEMScalarTimeAverageAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates a running time average of a scalar coefficient projected onto an auxvariable");
  params.addRequiredParam<MFEMScalarCoefficientName>("source", "Scalar coefficient to average");
  params.addParam<mfem::real_t>("time_skip", 0.0, "Time to skip before beginning the average");

  return params;
}

MFEMScalarTimeAverageAux::MFEMScalarTimeAverageAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_coefficient(getScalarCoefficient("source")),
    _result_coefficient(getScalarCoefficientByName(_result_var_name)),
    _average_var(_result_var.ParFESpace()),
    _skip(getParam<Real>("time_skip")),
    _time(getMFEMProblem().time()),
    _dt(getMFEMProblem().dt())
{
}

void
MFEMScalarTimeAverageAux::execute()
{
  if (_time <= _skip)
    return;

  // Linear blend: (1 - w) * avg_old + w * src
  const mfem::real_t w = _dt / (_time - _skip);
  mfem::SumCoefficient blend(_result_coefficient, _source_coefficient, 1.0 - w, w);
  _average_var.ProjectCoefficient(blend);

  _result_var = _average_var;
}

#endif
