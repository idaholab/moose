//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigenvaluesPostprocessor.h"
#include "MFEMEigenproblem.h"

registerMooseObject("MooseApp", MFEMEigenvaluesPostprocessor);

InputParameters
MFEMEigenvaluesPostprocessor::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();
  params.addClassDescription(
      "Retrieves the eigenvalues from an eigensolve for exporting.");
  return params;
}

MFEMEigenvaluesPostprocessor::MFEMEigenvaluesPostprocessor(const InputParameters & parameters)
  : MFEMVectorPostprocessor(parameters), _eigenvalues(this->declareVector("eigenvalues"))
{
}

void
MFEMEigenvaluesPostprocessor::initialize()
{
}

void
MFEMEigenvaluesPostprocessor::execute()
{
  auto eigensolver = std::dynamic_pointer_cast<MFEMEigensolverBase>(getMFEMProblem().getProblemData().jacobian_solver);
  if (!eigensolver)
    mooseError("The solver is not an eigensolver, cannot retrieve eigenvalues.");

  mfem::Array<mfem::real_t> eigenvalues;
  eigensolver->getEigenvalues(eigenvalues);

  const auto ev_dim = getMFEMProblem().getParam<int>("num_modes");
  _eigenvalues.get().resize(ev_dim);

  for (int i = 0; i < ev_dim; i++)
    _eigenvalues.get()[i] = eigenvalues[i];

  


}

#endif
