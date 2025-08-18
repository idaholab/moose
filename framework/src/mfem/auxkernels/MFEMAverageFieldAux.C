//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAverageFieldAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMAverageFieldAux);

InputParameters
MFEMAverageFieldAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Time-weighted running average of an MFEMVariable, projected into an AuxVariable.");
  params.addRequiredParam<VariableName>("source", "Name of the MFEMVariable to average from.");
  params.addParam<mfem::real_t>("timestep_skip", 0.0, "Time to skip before beginning the average.");

  return params;
}

MFEMAverageFieldAux::MFEMAverageFieldAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source_var_name)),
    _skip(getParam<mfem::real_t>("timestep_skip"))
//)
{
}

void
MFEMAverageFieldAux::execute()
{
  // project the average value using linear blend mfem::SumCoeffient calculated
  // from the current _source_var, with a time-average weight w = dt/(t-skip), and
  // return (1-w)*old + w*new for each integration point.
  mfem::real_t t = getMFEMProblem().time();
  mfem::real_t dt = getMFEMProblem().dt();
  // weight 0 until averaging starts, then dt/(t - _skip)
  const mfem::real_t w = (t > _skip) ? (dt / (t - _skip)) : 0.0;
  // Snapshot the previous average to avoid read/write aliasing during projection
  mfem::ParGridFunction old_copy(_result_var);
  // Wrap snapshot and source as coefficients
  mfem::GridFunctionCoefficient old_avg(&old_copy);
  mfem::GridFunctionCoefficient src(&_source_var);

  // Linear Blend (1-w)*old + w*src
  mfem::SumCoefficient blend(old_avg, src, (1.0 - w), w);

  _result_var.ProjectCoefficient(blend);
}

#endif
