//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// libmesh includes
#include "libmesh/petsc_vector.h"

// MOOSE includes
#include "NEML2PostKernel.h"

class NEML2StressDivergence : public NEML2PostKernel
{
public:
  static InputParameters validParams();

  NEML2StressDivergence(const InputParameters & parameters);

protected:
  /// calculate residual contribution corresponding to the weak form $\phi_{i,J} P_{iJ}$
  void forward() override;

  /// The residual vector
  PetscVector<Real> * _residual;

  /// stress
  const neml2::Tensor & _stress;

  /// Displacement variables
  const std::vector<NonlinearVariableName> _disp_vars;

  /// the number of displacement variables
  const int64_t _ndisp;

  /// test function gradients
  const neml2::Tensor * _grad_test_x;
  const neml2::Tensor * _grad_test_y;
  const neml2::Tensor * _grad_test_z;
};

#endif
