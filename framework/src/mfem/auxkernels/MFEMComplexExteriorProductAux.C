//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexExteriorProductAux.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("MooseApp", MFEMComplexExteriorProductAux);

InputParameters
MFEMComplexExteriorProductAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription(
      "Projects $s \\vec u \\wedge \\vec v*$ onto a complex vector MFEM auxvariable");
  params.addRequiredParam<VariableName>("first_source_vec", "Vector MFEMVariable U (vdim=3)");
  params.addRequiredParam<VariableName>("second_source_vec", "Vector MFEMVariable V (vdim=3)");
  params.addParam<mfem::real_t>(
      "scale_factor_real",
      1.0,
      "Real part of the constant multiplier applied to the cross product");
  params.addParam<mfem::real_t>(
      "scale_factor_imag",
      0.0,
      "Imaginary part of the constant multiplier applied to the cross product");

  return params;
}

MFEMComplexExteriorProductAux::MFEMComplexExteriorProductAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _scale_factor(getParam<mfem::real_t>("scale_factor_real"),
                  getParam<mfem::real_t>("scale_factor_imag")),
    _u_coef_real(getVectorCoefficientByName(getParam<VariableName>("first_source_vec") + "_real")),
    _u_coef_imag(getVectorCoefficientByName(getParam<VariableName>("first_source_vec") + "_imag")),
    _v_coef_real(getVectorCoefficientByName(getParam<VariableName>("second_source_vec") + "_real")),
    _v_coef_imag(getVectorCoefficientByName(getParam<VariableName>("second_source_vec") + "_imag")),
    _cross_ur_vr(_u_coef_real, _v_coef_real),
    _cross_ur_vi(_u_coef_real, _v_coef_imag),
    _cross_ui_vr(_u_coef_imag, _v_coef_real),
    _cross_ui_vi(_u_coef_imag, _v_coef_imag),
    _final_coef_real(
        _cross_ur_vr, _cross_ui_vi, 1.0, 1.0), // Taking into account hermitian conjugation
    _final_coef_imag(_cross_ur_vi, _cross_ui_vr, -1.0, 1.0)
{
  // Check the target variable type and dimensions
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Enforce 3D cross product
  if (fes->GetVDim() != 3)
    mooseError("MFEMComplexExteriorProductAux requires a 3D mesh (Dimension == 3).");

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError(
        "MFEMComplexExteriorProductAux requires the target variable to use L2_FECollection.");

  if (fes->GetVDim() != 3)
    mooseError("MFEMComplexExteriorProductAux requires AuxVariable to have vdim == 3.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMComplexExteriorProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMComplexExteriorProductAux::execute()
{

  // MFEM element projection for L2
  _result_var.real().ProjectCoefficient(_final_coef_real);
  _result_var.imag().ProjectCoefficient(_final_coef_imag);

  complexScale(_result_var, _scale_factor);
}

#endif // MOOSE_MFEM_ENABLED
