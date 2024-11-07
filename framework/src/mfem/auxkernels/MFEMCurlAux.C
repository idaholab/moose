#pragma once
#include "MFEMCurlAux.h"

registerMooseObject("PlatypusApp", MFEMCurlAux);

/*
Class to set an auxvariable to be the curl of a variable.
*/
InputParameters
MFEMCurlAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addRequiredParam<VariableName>("source",
                                        "Vector H(curl) MFEMVariable to take the curl of.");
  return params;
}

MFEMCurlAux::MFEMCurlAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData()._gridfunctions.Get(_source_var_name)),
    _hcurl_fespace(*_source_var.ParFESpace()),
    _hdiv_fespace(*_result_var.ParFESpace()),
    _curl(&_hcurl_fespace, &_hdiv_fespace)
{
  _curl.AddDomainInterpolator(new mfem::CurlInterpolator);
  _curl.Assemble();
  _curl.Finalize();
}

// Computes the auxvariable.
void
MFEMCurlAux::execute()
{
  _curl.Mult(_source_var, _result_var);
}
