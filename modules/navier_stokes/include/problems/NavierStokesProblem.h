//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"
#include <petscsnes.h>

class NonlinearSystem;

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class NavierStokesProblem : public FEProblem
{
public:
  static InputParameters validParams();

  NavierStokesProblem(const InputParameters & parameters);

  TagID massMatrixTagID() const { return getMatrixTagID(_mass_matrix); }
  Mat & getQscale() { return _Q_scale; }

  void clearIndexSets() { _index_sets.clear(); }
  KSP findSchurKSP(KSP node, unsigned int tree_position);
  void setupLSCMatrices(KSP schur_ksp);

  virtual ~NavierStokesProblem();

protected:
  /**
   * Reinitialize PETSc output for proper linear/nonlinear iteration display
   */
  virtual void initPetscOutput() override;

private:
  const TagName & _mass_matrix;
  const std::vector<unsigned int> & _schur_fs_index;

  Mat _Q_scale = nullptr;
  std::vector<IS> _index_sets;
};
