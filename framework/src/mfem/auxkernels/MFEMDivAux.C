//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDivAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDivAux);

InputParameters
MFEMDivAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the divergence of an H(div) conforming RT source variable and stores the result"
      " on an L2 conforming result auxvariable");
  params.addRequiredParam<VariableName>("source",
                                        "Vector H(div) MFEMVariable to take the divergence of.");
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

MFEMDivAux::MFEMDivAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _div(_source_var.ParFESpace(), _result_var.ParFESpace())
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

#endif
