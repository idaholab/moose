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
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

/// Modified version of the MFEM backward Euler solver solving for the state of
/// the time dependent variable at the next timestep directly, instead of the
/// time derivative.
///
/// To be replaced if/when https://github.com/mfem/mfem/pull/5079 is merged.
class MFEMBackwardEulerStateSolver : public mfem::ODESolver
{
protected:
  mfem::Vector k;

public:
  void Init(mfem::TimeDependentOperator & f_) override;

  void Step(mfem::Vector & x, mfem::real_t & t, mfem::real_t & dt) override;
};

}

#endif
