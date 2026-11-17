//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMIntegratedBC.h"
#include "MFEMProblem.h"

InputParameters
MFEMIntegratedBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  return params;
}

MFEMIntegratedBC::MFEMIntegratedBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters)
{
}

bool
MFEMIntegratedBC::isDGBC() const
{
  // A discontinuous space has no degrees of freedom on boundary elements, so a BC acting on one
  // is integrated over the boundary faces of the elements instead.
  const auto & test_gf = getMFEMProblem().getProblemData().gridfunctions.GetRef(_test_var_name);
  return test_gf.ParFESpace()->FEColl()->GetContType() ==
         mfem::FiniteElementCollection::DISCONTINUOUS;
}

#endif
