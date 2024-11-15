#pragma once
#include "MFEMGradAux.h"

registerMooseObject("PlatypusApp", MFEMGradAux);

/*
Class to set an H(curl) auxvariable to be the gradient of a H1 scalar variable.
*/
InputParameters
MFEMGradAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addRequiredParam<VariableName>("source",
                                        "Scalar H1 MFEMVariable to take the gradient of.");
  return params;
}

MFEMGradAux::MFEMGradAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData()._gridfunctions.Get(_source_var_name)),
    _h1_fespace(*_source_var.ParFESpace()),
    _hcurl_fespace(*_result_var.ParFESpace()),
    _grad(&_h1_fespace, &_hcurl_fespace)
{
  _grad.Assemble();
  _grad.Finalize();
}

// Computes the auxvariable.
void
MFEMGradAux::execute()
{
  _grad.Mult(_source_var, _result_var);
}
