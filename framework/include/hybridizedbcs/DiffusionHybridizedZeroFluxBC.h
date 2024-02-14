//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HybridizedIntegratedBC.h"
#include "DiffusionHybridizedInterface.h"

#include <vector>

template <typename>
class MooseArray;
class Function;

/**
 * Implements a zero-flux boundary condition for use with a hybridized discretization of the
 * diffusion equation
 */
class DiffusionHybridizedZeroFluxBC : public HybridizedIntegratedBC,
                                      public DiffusionHybridizedInterface
{
public:
  static InputParameters validParams();

  DiffusionHybridizedZeroFluxBC(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onBoundary() override;

  /// transformed Jacobian weights on the face
  const MooseArray<Real> & _JxW_face;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  friend class DiffusionHybridizedKernel;
  friend class DiffusionHybridizedInterface;
};
