//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMInnerProductAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMInnerProductAux);

InputParameters
MFEMInnerProductAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects $s \\vec u \\cdot \\vec v$ onto a scalar MFEM auxvariable");
  params.addRequiredParam<MFEMVectorCoefficientName>("first_source_vec", "Vector coefficient");
  params.addRequiredParam<MFEMVectorCoefficientName>("second_source_vec", "Vector coefficient");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient s to scale the inner product by");
  return params;
}

MFEMInnerProductAux::MFEMInnerProductAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _inner(getVectorCoefficient("first_source_vec"), getVectorCoefficient("second_source_vec")),
    _scaled_inner(getScalarCoefficient("coefficient"), _inner)
{
  // The target variable's finite element space
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Must be L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()))
    mooseError("MFEMInnerProductAux requires the target variable to use L2_FECollection.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("MFEMInnerProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
MFEMInnerProductAux::execute()
{
  _result_var.ProjectCoefficient(_scaled_inner);
}

#endif // MOOSE_MFEM_ENABLED
