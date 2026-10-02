//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

/// Patched version of mfem::ParMixedBilinearForm to extend support to cases where MFEM trial variable is defined on a submesh of the MFEM test variable
class ParMixedBilinearForm : public mfem::ParMixedBilinearForm
{

public:
  ParMixedBilinearForm(mfem::ParFiniteElementSpace * tr_fes, mfem::ParFiniteElementSpace * te_fes)
    : mfem::ParMixedBilinearForm(tr_fes, te_fes)
  {
  }

  ParMixedBilinearForm(mfem::ParFiniteElementSpace * tr_fes,
                       mfem::ParFiniteElementSpace * te_fes,
                       mfem::ParMixedBilinearForm * mbf)
    : mfem::ParMixedBilinearForm(tr_fes, te_fes, mbf)
  {
  }

  // Re-implementation of Assemble to maintain functionality when the trial variable is defined on a
  // submesh of the test variable. Defaults to mfem::ParMixedBilinearForm::Assemble otherwise.
  void Assemble(int skip_zeros = 1);
  // Custom implementation of Assemble for use when the trial variable is defined on a submesh of
  // the test variable
  void SubMeshTolerantAssemble(int skip_zeros);
};
} // namespace Moose::MFEM

#endif
