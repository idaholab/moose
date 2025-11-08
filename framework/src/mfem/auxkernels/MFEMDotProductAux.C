//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDotProductAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDotProductAux);

InputParameters
MFEMDotProductAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Project s(x) * (U . V) onto a scalar MFEM auxvariable.");
  params.addRequiredParam<VariableName>("first_source_vec", "Vector MFEMVariable U");
  params.addRequiredParam<VariableName>("second_source_vec", "Vector MFEMVariable V");
  params.addParam<mfem::real_t>(
      "scale_factor", 1.0, "Constant multiplier applied to the dot product");
  return params;
}

MFEMDotProductAux::MFEMDotProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _u_var_name(getParam<VariableName>("first_source_vec")),
    _v_var_name(getParam<VariableName>("second_source_vec")),
    _u_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_u_var_name)),
    _v_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _u_coef(&_u_var),
    _v_coef(&_v_var),
    _dot_uv(_u_coef, _v_coef),
    _scale_c(_scale_factor),
    _final_coef(_scale_c, _dot_uv)
{
  // Must be L2
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    paramError("variable", "The target variable must use L2_FECollection.");

  // Must have no shared/constrained DOFs
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMDotProductAux currently supports only L2 spaces with interior DOFs (no "
               "shared/constrained DOFs).");
}

void
MFEMDotProductAux::execute()
{
  // Project into the scalar aux result variable per element projection for L2
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_final_coef);
}

#endif // MOOSE_MFEM_ENABLED
