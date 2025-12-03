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

namespace Moose::MFEM
{

/// Modified version of the MFEM backward Euler solver solving for the state of
/// the time dependent variable at the next timestep directly, instead of the
/// time derivative.
///
/// To be replaced if/when https://github.com/mfem/mfem/pull/5079 is merged.
class MFEMBackwardEulerStateSolver : public mfem::BackwardEulerSolver
{
public:
  void Step(mfem::Vector & x, mfem::real_t & t, mfem::real_t & dt) override
  {
    f->SetTime(t + dt);
    f->ImplicitSolve(dt, x, k); // solve for k = x(t + dt)
    x = k;
    t += dt;
  }
};
}

#endif
