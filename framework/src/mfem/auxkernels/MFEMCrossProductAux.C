//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCrossProductAux.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("MooseApp", MFEMCrossProductAux);

InputParameters
MFEMCrossProductAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects s(x) * (U x V) onto a vector MFEM auxvariable");
  params.addRequiredParam<VariableName>("first_source_vec", "Vector MFEMVariable U (vdim=3)");
  params.addRequiredParam<VariableName>("second_source_vec", "Vector MFEMVariable V (vdim=3)");
  params.addParam<mfem::real_t>(
      "scale_factor", 1.0, "Constant multiplier applied to the cross product");
  return params;
}

MFEMCrossProductAux::MFEMCrossProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _u_var_name(getParam<VariableName>("first_source_vec")),
    _v_var_name(getParam<VariableName>("second_source_vec")),
    _u_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_u_var_name)),
    _v_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor")),
    _u_coef(&_u_var),
    _v_coef(&_v_var),
    _cross_uv(_u_coef, _v_coef),
    _scale_c(_scale_factor),
    _final_coef(_scale_c, _cross_uv)
{
  // Check the target variable type and dimensions
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();
  const int mesh_dim = fes->GetMesh()->Dimension();

  // Enforce 3D cross product
  if (mesh_dim != 3)
    mooseError("MFEMCrossProductAux requires a 3D mesh (Dimension == 3).");

  if (fes->GetVDim() != 3)
    mooseError("MFEMCrossProductAux requires AuxVariable to have vdim == 3.");

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMCrossProductAux requires the target variable to use L2_FECollection.");

  // // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMCrossProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMCrossProductAux::execute()
{

  // MFEM element projection for L2
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_final_coef);
}

#endif // MOOSE_MFEM_ENABLED
