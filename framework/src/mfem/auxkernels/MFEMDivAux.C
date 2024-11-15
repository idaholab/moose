#pragma once
#include "MFEMDivAux.h"

registerMooseObject("PlatypusApp", MFEMDivAux);

/*
Class to set an L2 auxvariable to be the divergence of a H(div) vector variable.
*/
InputParameters
MFEMDivAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addRequiredParam<VariableName>("source",
                                        "Vector H(div) MFEMVariable to take the divergence of.");
  return params;
}

MFEMDivAux::MFEMDivAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData()._gridfunctions.Get(_source_var_name)),
    _hdiv_fespace(*_source_var.ParFESpace()),
    _l2_fespace(*_result_var.ParFESpace()),
    _div(&_hdiv_fespace, &_l2_fespace)
{
  _div.Assemble();
  _div.Finalize();
}

// Computes the auxvariable.
void
MFEMDivAux::execute()
{
  _div.Mult(_source_var, _result_var);
}
