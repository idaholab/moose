//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexCrossProductAux.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("MooseApp", MFEMComplexCrossProductAux);

InputParameters
MFEMComplexCrossProductAux::validParams()
{
  InputParameters params = MFEMComplexAuxKernel::validParams();
  params.addClassDescription("Projects s(x) * (U x V) onto a vector MFEM auxvariable");
  params.addRequiredParam<VariableName>("first_source_vec", "Vector MFEMVariable U (vdim=3)");
  params.addRequiredParam<VariableName>("second_source_vec", "Vector MFEMVariable V (vdim=3)");
  params.addParam<mfem::real_t>(
      "scale_factor_real", 1.0, "Real part of the constant multiplier applied to the cross product");
  params.addParam<mfem::real_t>(
      "scale_factor_imag", 0.0, "Imaginary part of the constant multiplier applied to the cross product");

  return params;
}

MFEMComplexCrossProductAux::MFEMComplexCrossProductAux(const InputParameters & parameters)
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
    _cross_ur_vr(_u_coef_real, _v_coef_real),
    _cross_ur_vi(_u_coef_real, _v_coef_imag),
    _cross_ui_vr(_u_coef_imag, _v_coef_real),
    _cross_ui_vi(_u_coef_imag, _v_coef_imag),
    _final_coef_real(_cross_ur_vr, _cross_ui_vi, 1.0, -1.0),
    _final_coef_imag(_cross_ur_vi, _cross_ui_vr, 1.0, 1.0)
{
  // Check the target variable type and dimensions
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();
  const int mesh_dim = fes->GetMesh()->Dimension();

  // Enforce 3D cross product
  if (mesh_dim != 3)
    mooseError("MFEMComplexCrossProductAux requires a 3D mesh (Dimension == 3).");

  if (fes->GetVDim() != 3)
    mooseError("MFEMComplexCrossProductAux requires AuxVariable to have vdim == 3.");

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMComplexCrossProductAux requires the target variable to use L2_FECollection.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMComplexCrossProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMComplexCrossProductAux::execute()
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
