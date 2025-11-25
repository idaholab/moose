//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexDotProductAux.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("MooseApp", MFEMComplexDotProductAux);

InputParameters
MFEMComplexDotProductAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription("Projects s(x) * (U x V) onto a complex vector MFEM auxvariable");
  params.addRequiredParam<VariableName>("first_source_vec",
                                        "Complex vector MFEMVariable U (vdim=3)");
  params.addRequiredParam<VariableName>("second_source_vec",
                                        "Complex vector MFEMVariable V (vdim=3)");
  params.addParam<mfem::real_t>(
      "scale_factor_real", 1.0, "Real part of the constant multiplier applied to the dot product");
  params.addParam<mfem::real_t>(
      "scale_factor_imag",
      0.0,
      "Imaginary part of the constant multiplier applied to the dot product");

  return params;
}

MFEMComplexDotProductAux::MFEMComplexDotProductAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _u_var_name(getParam<VariableName>("first_source_vec")),
    _v_var_name(getParam<VariableName>("second_source_vec")),
    _u_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_u_var_name)),
    _v_var(*getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(_v_var_name)),
    _scale_factor_real(getParam<mfem::real_t>("scale_factor_real")),
    _scale_factor_imag(getParam<mfem::real_t>("scale_factor_imag")),
    _u_coef_real(&_u_var.real()),
    _u_coef_imag(&_u_var.imag()),
    _v_coef_real(&_v_var.real()),
    _v_coef_imag(&_v_var.imag()),
    _dot_ur_vr(_u_coef_real, _v_coef_real),
    _dot_ur_vi(_u_coef_real, _v_coef_imag),
    _dot_ui_vr(_u_coef_imag, _v_coef_real),
    _dot_ui_vi(_u_coef_imag, _v_coef_imag),
    _final_coef_real(_dot_ur_vr, _dot_ui_vi, 1.0, 1.0), // Taking into account hermitian conjugation
    _final_coef_imag(_dot_ur_vi, _dot_ui_vr, -1.0, 1.0)
{
  // Check the target variable type and dimensions
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();
  if (fes->GetVDim() != 1)
    mooseError("MFEMComplexDotProductAux requires the target variable to be a scalar (vdim=1).");
    
  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMComplexDotProductAux requires the target variable to use L2_FECollection.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMComplexDotProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMComplexDotProductAux::execute()
{

  // MFEM element projection for L2
  _result_var.real() = 0.0;
  _result_var.imag() = 0.0;
  _result_var.real().ProjectCoefficient(_final_coef_real);
  _result_var.imag().ProjectCoefficient(_final_coef_imag);

  std::complex<mfem::real_t> scale_complex(_scale_factor_real, _scale_factor_imag);
  complexScale(_result_var, scale_complex);
}

#endif // MOOSE_MFEM_ENABLED
