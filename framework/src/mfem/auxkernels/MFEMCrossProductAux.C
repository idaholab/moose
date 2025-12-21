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

registerMooseObject("MooseApp", MFEMCrossProductAux);

InputParameters
MFEMCrossProductAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects $s \\vec u \\times \\vec v$ onto a vector MFEM auxvariable");
  params.addRequiredParam<MFEMVectorCoefficientName>("first_source_vec", "Vector coeff (3D)");
  params.addRequiredParam<MFEMVectorCoefficientName>("second_source_vec", "Vector coeff (3D)");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient s to scale the cross product by");
  return params;
}

MFEMCrossProductAux::MFEMCrossProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _cross(getVectorCoefficient("first_source_vec"), getVectorCoefficient("second_source_vec")),
    _scaled_cross(getScalarCoefficient("coefficient"), _cross)
{
  // The target variable's finite element space
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Must be [L2]^3
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()) || fes->GetVDim() != 3)
    mooseError("MFEMCrossProductAux requires the target variable to be vector [L2]^3.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMCrossProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMCrossProductAux::execute()
{
  _result_var.ProjectCoefficient(_scaled_cross);
}

#endif // MOOSE_MFEM_ENABLED
