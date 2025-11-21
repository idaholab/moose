//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexCurlAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexCurlAux);

InputParameters
MFEMComplexCurlAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the curl of a complex H(curl) conforming ND source variable and stores the result"
      " on an H(div) conforming RT result complex auxvariable");
  params.addRequiredParam<VariableName>("source",
                                        "Vector H(curl) MFEMComplexVariable to take the curl of.");
  params.addParam<mfem::real_t>("scale_factor_real", 1.0, "Real part of the factor to scale result auxvariable by.");
  params.addParam<mfem::real_t>("scale_factor_imag", 0.0, "Imaginary part of the factor to scale result auxvariable by.");
  return params;
}

MFEMComplexCurlAux::MFEMComplexCurlAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_source_var_name)),
    _scale_factor_real(getParam<mfem::real_t>("scale_factor_real")),
    _scale_factor_imag(getParam<mfem::real_t>("scale_factor_imag")),
    _curl(_source_var.real().ParFESpace(), _result_var.real().ParFESpace())
{
  _curl.Assemble();
  _curl.Finalize();
}

// Computes the auxvariable.
void
MFEMComplexCurlAux::execute()
{
  _result_var.real() = 0.0;
  _result_var.imag() = 0.0;
  _curl.AddMult(_source_var.real(), _result_var.real());
  _curl.AddMult(_source_var.imag(), _result_var.imag());

  std::complex<mfem::real_t> scale_complex(_scale_factor_real, _scale_factor_imag);
  complexScale(_result_var, scale_complex);
}

#endif
