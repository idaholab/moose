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

registerMooseMFEMObject("MooseApp", InnerProductAux);

namespace Moose::MFEM
{
InputParameters
InnerProductAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Projects $s \\vec u \\cdot \\vec v$ onto a scalar MFEM auxvariable");
  params.addRequiredParam<Moose::MFEM::VectorCoefficientName>("first_source_vec",
                                                              "Vector coefficient");
  params.addRequiredParam<Moose::MFEM::VectorCoefficientName>("second_source_vec",
                                                              "Vector coefficient");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient s to scale the inner product by");
  return params;
}

InnerProductAux::InnerProductAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _inner(getVectorCoefficient("first_source_vec"), getVectorCoefficient("second_source_vec")),
    _scaled_inner(getScalarCoefficient("coefficient"), _inner)
{
  // The target variable's finite element space
  mfem::ParFiniteElementSpace * fes = _result_var.ParFESpace();

  // Must be scalar L2
  if (!dynamic_cast<const mfem::L2_FECollection *>(fes->FEColl()) || fes->GetVDim() != 1)
    mooseError("Moose::MFEM::InnerProductAux requires the target variable to be scalar L2.");

  // Must have no shared/constrained DOFs (pure interior DOFs)
  if (fes->GetTrueVSize() != fes->GetVSize())
    mooseError("Moose::MFEM::InnerProductAux currently supports only L2 spaces with interior DOFs "
               "(no shared/constrained DOFs).");
}

void
InnerProductAux::execute()
{
  _result_var.ProjectCoefficient(_scaled_inner);
}

} // namespace Moose::MFEM
#endif // MOOSE_MFEM_ENABLED
