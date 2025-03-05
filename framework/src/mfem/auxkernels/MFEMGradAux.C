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
  params.addClassDescription(
      "Calculates the gradient of an H1 conforming source variable and stores the result"
      " on an H(curl) conforming ND result auxvariable");
  params.addRequiredParam<VariableName>("source",
                                        "Scalar H1 MFEMVariable to take the gradient of.");
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

MFEMGradAux::MFEMGradAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _grad(_source_var.ParFESpace(), _result_var.ParFESpace())
{
  _grad.Assemble();
  _grad.Finalize();
}

// Computes the auxvariable.
void
MFEMGradAux::execute()
{
  _result_var = 0.0;
  _grad.AddMult(_source_var, _result_var, _scale_factor);
}
