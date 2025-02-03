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
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

MFEMDivAux::MFEMDivAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData()._gridfunctions.Get(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
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
  _result_var = 0.0;
  _div.AddMult(_source_var, _result_var, _scale_factor);
}
