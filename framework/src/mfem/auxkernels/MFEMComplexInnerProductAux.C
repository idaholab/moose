//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexInnerProductAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexInnerProductAux);

InputParameters
MFEMComplexInnerProductAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription(
      "Projects $s \\vec u \\cdot \\vec v*$ onto a complex scalar MFEM auxvariable");
  params.addRequiredParam<VariableName>("first_source_vec", "Complex vector variable");
  params.addRequiredParam<VariableName>("second_source_vec", "Complex vector variable");
  params.addParam<mfem::real_t>("scale_factor_real", 1.0, "Real part of constant multiplier");
  params.addParam<mfem::real_t>("scale_factor_imag", 0.0, "Imaginary part of constant multiplier");

  return params;
}

MFEMComplexInnerProductAux::MFEMComplexInnerProductAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _scale_factor(getParam<mfem::real_t>("scale_factor_real"),
                  getParam<mfem::real_t>("scale_factor_imag")),
    _u_coef_real(getVectorCoefficientByName(getParam<VariableName>("first_source_vec") + "_real")),
    _u_coef_imag(getVectorCoefficientByName(getParam<VariableName>("first_source_vec") + "_imag")),
    _v_coef_real(getVectorCoefficientByName(getParam<VariableName>("second_source_vec") + "_real")),
    _v_coef_imag(getVectorCoefficientByName(getParam<VariableName>("second_source_vec") + "_imag")),
    _dot_ur_vr(_u_coef_real, _v_coef_real),
    _dot_ur_vi(_u_coef_real, _v_coef_imag),
    _dot_ui_vr(_u_coef_imag, _v_coef_real),
    _dot_ui_vi(_u_coef_imag, _v_coef_imag),
    _final_coef_real(_dot_ur_vr, _dot_ui_vi, 1.0, 1.0), // Accounting for hermitian conjugation
    _final_coef_imag(_dot_ur_vi, _dot_ui_vr, -1.0, 1.0)
{
  // The target variable's finite element space
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Must be scalar L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()) || fes->GetVDim() != 1)
    mooseError("MFEMComplexInnerProductAux requires the target variable to be scalar L2.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMComplexInnerProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMComplexInnerProductAux::execute()
{
  _result_var.ProjectCoefficient(_final_coef_real, _final_coef_imag);

  complexScale(_result_var, _scale_factor);
}

#endif // MOOSE_MFEM_ENABLED
