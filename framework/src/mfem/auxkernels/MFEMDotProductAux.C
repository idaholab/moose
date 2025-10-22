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
  params.addRequiredParam<VariableName>("u", "Vector MFEMVariable U");
  params.addRequiredParam<VariableName>("v", "Vector MFEMVariable V");
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Constant multiplier applied to the dot product");
  // params.addParam<VariableName>("scale_variable", "Optional scalar MFEMVariable s(x) to multiply the dot product.");
  return params;
}

MFEMDotProductAux::MFEMDotProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _u_var_name(getParam<VariableName>("u")),
    _v_var_name(getParam<VariableName>("v")),
    _u_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_u_var_name)),
    _v_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v_var_name)),
    // _scale_var(parameters.isParamValid("scale_variable")
                //  ? &(*getMFEMProblem().getProblemData().gridfunctions.Get(
                      //  getParam<VariableName>("scale_variable")))
                //  : nullptr),
    _scale_factor(getParam<mfem::real_t>("scale_factor"))
{
  // nothing else to assemble, the coefficient expression is build in the execute()
}

void
MFEMDotProductAux::execute()
{
  // Build scalar coefficient s(x) * (U . V)
  mfem::VectorGridFunctionCoefficient Ucoef(const_cast<mfem::ParGridFunction *>(&_u_var));
  mfem::VectorGridFunctionCoefficient Vcoef(const_cast<mfem::ParGridFunction *>(&_v_var));
  mfem::InnerProductCoefficient dot_uv(Ucoef, Vcoef);

  // If you enable scale_variable, multiply by that GridFunctionCoefficient here.
  mfem::ConstantCoefficient ccoef(_scale_factor);
  mfem::ProductCoefficient final_coef(ccoef, dot_uv);

  //Enforce L2 with INTEGRAL mapping and interior DOFs only
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMDotProductAux requires the target variable to use L2_FECollection.");

  // Must use integral mapping (element-integrated DOFs / quadrature-point-based DOFs)
  const int dim = fes->GetMesh()->Dimension();
  const auto mt = static_cast<mfem::FiniteElement::MapType>(fes->FEColl()->GetMapType(dim));
  if (mt != mfem::FiniteElement::INTEGRAL)
    mooseError("MFEMDotProductAux requires map_type = FiniteElement::INTEGRAL for the target L2 space.");

  // Must have no shared/constrained DOFs
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMDotProductAux currently supports only L2 spaces with interior DOFs (no shared/constrained DOFs).");


  // Project into the scalar aux result variable per element projection for L2
  _result_var = 0.0;
  _result_var.ProjectCoefficient(final_coef);
}

#endif // MOOSE_MFEM_ENABLED
