//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMBackwardEulerStateSolver.h"

namespace Moose::MFEM
{

void
MFEMBackwardEulerStateSolver::Init(mfem::TimeDependentOperator & f_)
{
  mfem::ODESolver::Init(f_);
  k.SetSize(f->Width(), mem_type);
}

void
MFEMBackwardEulerStateSolver::Step(mfem::Vector & x, mfem::real_t & t, mfem::real_t & dt)
{
  f->SetTime(t + dt);
  f->ImplicitSolve(dt, x, k); // solve for k: k = f(x + dt*k, t + dt)
  x = k;                      // x = u_{i+1}
  t += dt;
}

}

#endif
