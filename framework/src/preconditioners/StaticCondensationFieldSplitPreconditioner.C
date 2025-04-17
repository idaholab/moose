//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StaticCondensationFieldSplitPreconditioner.h"

// MOOSE includes
#include "PetscSupport.h"
#include "NonlinearSystemBase.h"

#include "libmesh/static_condensation.h"

registerMooseObjectAliased("MooseApp", StaticCondensationFieldSplitPreconditioner, "SCFSP");

InputParameters
StaticCondensationFieldSplitPreconditioner::validParams()
{
  return FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>::validParams();
}

StaticCondensationFieldSplitPreconditioner::StaticCondensationFieldSplitPreconditioner(
    const InputParameters & params)
  : FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>(params)
{
  // set a top splitting
  _nl.setFieldSplitData(_top_split, *_sc);

  // apply prefix and store PETSc options
  _nl.setupFieldDecomposition();
}
