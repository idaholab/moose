//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// To be removed as soon as we have a fix upstream in either hypre or mfem

#ifdef MOOSE_MFEM_ENABLED

namespace mfem
{
namespace patched
{
/**
 * Patch for mfem::HypreGMRES to reset preconditioning matrix at every nonlinear/time iteration
 */
class HypreGMRES : public mfem::HypreGMRES
{
public:
  using mfem::HypreGMRES::HypreGMRES;
  void SetOperator(const mfem::Operator & op)
  {
    mfem::HypreGMRES::SetOperator(op);
    HYPRE_GMRESSetPrecondMatrix(HYPRE_Solver(*this), nullptr);
  }
};

/**
 * Patch for mfem::HyprePCG to reset preconditioning matrix at every nonlinear/time iteration
 */
class HyprePCG : public mfem::HyprePCG
{
public:
  using mfem::HyprePCG::HyprePCG;
  void SetOperator(const mfem::Operator & op)
  {
    mfem::HyprePCG::SetOperator(op);
    HYPRE_PCGSetPrecondMatrix(HYPRE_Solver(*this), nullptr);
  }
};
}
}

#endif
