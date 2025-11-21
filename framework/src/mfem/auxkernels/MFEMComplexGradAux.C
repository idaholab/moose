//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexGradAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexGradAux);

InputParameters
MFEMComplexGradAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the gradient of an H1 conforming source variable and stores the result"
      " on an H(curl) conforming ND result auxvariable");
  params.addRequiredParam<VariableName>("source",
                                        "Scalar H1 MFEMVariable to take the gradient of.");
  params.addParam<mfem::real_t>(
      "scale_factor_real", 1.0, "Real part of the factor to scale result auxvariable by.");
  params.addParam<mfem::real_t>(
      "scale_factor_imag", 0.0, "Imaginary part of the factor to scale result auxvariable by.");
  return params;
}

MFEMComplexGradAux::MFEMComplexGradAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_source_var_name)),
    _scale_factor_real(getParam<mfem::real_t>("scale_factor_real")),
    _scale_factor_imag(getParam<mfem::real_t>("scale_factor_imag")),
    _grad(_source_var.real().ParFESpace(), _result_var.real().ParFESpace())
{
  _grad.Assemble();
  _grad.Finalize();
}

// Computes the auxvariable.
void
MFEMComplexGradAux::execute()
{
  _result_var.real() = 0;
  _result_var.imag() = 0;
  _grad.AddMult(_source_var.real(), _result_var.real());
  _grad.AddMult(_source_var.imag(), _result_var.imag());

  std::complex<mfem::real_t> scale_complex(_scale_factor_real, _scale_factor_imag);
  complexScale(_result_var, scale_complex);
}

#endif
