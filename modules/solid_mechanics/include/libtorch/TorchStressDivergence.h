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
#include "TorchPostKernel.h"

class TorchStressDivergence : public TorchPostKernel
{
public:
  static InputParameters validParams();

  TorchStressDivergence(const InputParameters & parameters);

protected:
  /// calculate residual contribution corresponding to the weak form $\phi_{i,J} P_{iJ}$
  void forward() override;

  /// The residual vector
  libMesh::PetscVector<Real> * _residual;

  /// stress (a NEML2 model output, 6-component Mandel)
  const at::Tensor & _stress;

  /// Displacement variables
  const std::vector<NonlinearVariableName> _disp_vars;

  /// the number of displacement variables
  const int64_t _ndisp;

  /// test function gradients
  const at::Tensor * _grad_test_x;
  const at::Tensor * _grad_test_y;
  const at::Tensor * _grad_test_z;
};

#endif // NEML2_ENABLED
