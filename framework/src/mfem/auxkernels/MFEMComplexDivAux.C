//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexDivAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexDivAux);

InputParameters
MFEMComplexDivAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription("Calculates the divergence of a complex H(div) conforming RT source "
                             "variable and stores the result"
                             " on an L2 conforming result complex auxvariable");
  params.addRequiredParam<VariableName>(
      "source", "Vector H(div) MFEMComplexVariable to take the divergence of.");
  params.addParam<mfem::real_t>(
      "scale_factor_real", 1.0, "Real part of the factor to scale result auxvariable by.");
  params.addParam<mfem::real_t>(
      "scale_factor_imag", 0.0, "Imaginary part of the factor to scale result auxvariable by.");

  return params;
}

MFEMComplexDivAux::MFEMComplexDivAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_source_var_name)),
    _scale_factor_real(getParam<mfem::real_t>("scale_factor_real")),
    _scale_factor_imag(getParam<mfem::real_t>("scale_factor_imag")),
    _div(_source_var.real().ParFESpace(), _result_var.real().ParFESpace())
{
  _div.Assemble();
  _div.Finalize();
}

// Computes the auxvariable.
void
MFEMComplexDivAux::execute()
{
  _div.AddMult(_source_var.real(), _result_var.real() = 0);
  _div.AddMult(_source_var.imag(), _result_var.imag() = 0);

  std::complex<mfem::real_t> scale_complex(_scale_factor_real, _scale_factor_imag);
  complexScale(_result_var, scale_complex);
}

#endif
