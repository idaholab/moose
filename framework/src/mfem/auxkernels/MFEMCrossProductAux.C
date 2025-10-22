#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCrossProductAux.h"
#include "MFEMProblem.h"
#include "mfem.hpp"

registerMooseObject("MooseApp", MFEMCrossProductAux);

InputParameters
MFEMCrossProductAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects s(x) * (U × V) onto a vector MFEM auxvariable");
  params.addRequiredParam<VariableName>("u", "Vector MFEMVariable U (vdim=3)");
  params.addRequiredParam<VariableName>("v", "Vector MFEMVariable V (vdim=3)");
  params.addParam<mfem::real_t>(
      "scale_factor", 1.0, "Constant multiplier applied to the cross product");
  return params;
}

MFEMCrossProductAux::MFEMCrossProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _u_var_name(getParam<VariableName>("u")),
    _v_var_name(getParam<VariableName>("v")),
    _u_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_u_var_name)),
    _v_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor"))
{
}

void MFEMCrossProductAux::execute()
{
  // Build vector coefficient: s(x) * (U × V)
  mfem::VectorGridFunctionCoefficient Ucoef(const_cast<mfem::ParGridFunction *>(&_u_var));
  mfem::VectorGridFunctionCoefficient Vcoef(const_cast<mfem::ParGridFunction *>(&_v_var));
  mfem::VectorCrossProductCoefficient cross_uv(Ucoef, Vcoef); // vector-valued

  // s(x) = constant scale factor for now
  mfem::ConstantCoefficient scoef(_scale_factor);
  mfem::ScalarVectorProductCoefficient final_vec(scoef, cross_uv); // vector-valued

  //Must be vector L2 with INTEGRAL map and interior DOFs
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();
  const int mesh_dim = fes->GetMesh()->Dimension();

  //Enforce 3D cross product
  if (mesh_dim != 3)
    mooseError("MFEMCrossProductAux requires a 3D mesh (Dimension == 3).");

  if (fes->GetVDim() != 3)
    mooseError("MFEMCrossProductAux requires AuxVariable to have vdim == 3.");

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMCrossProductAux requires the target variable to use L2_FECollection.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMCrossProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");

  //MFEM element projection for L2
  _result_var = 0.0;
  _result_var.ProjectCoefficient(final_vec);
}

#endif // MOOSE_MFEM_ENABLED