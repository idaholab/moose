//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGradAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGradAux);

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

#endif
