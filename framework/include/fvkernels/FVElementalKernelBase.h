//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVKernel.h"
#include "MooseVariableFV.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"

/// FVElemental is used for calculating residual contributions from volume
/// integral terms of a PDE where the divergence theorem is not applied (e.g.
/// time derivative terms, source terms, etc.).  As with finite element
/// kernels, all solution values and material properties must be indexed using
/// the _qp member.  Note that all interfaces for finite volume kernels are
/// AD-based - be sure to use AD material properties and other AD values to
/// maintain good jacobian/derivative quality.
class FVElementalKernelBase : public FVKernel,
                              public CoupleableMooseVariableDependencyIntermediateInterface,
                              public MaterialPropertyInterface
{
public:
  static InputParameters validParams();
  FVElementalKernelBase(const InputParameters & parameters);

  /// These function should be generallhy overriden by the intermediate child classes only -
  /// i.e. regular and array kernel base classes - and not overriden by final kernel
  /// implementationsw.
  ///@{
  virtual void computeResidual() = 0;
  virtual void computeJacobian() = 0;
  ///@}

protected:
  const unsigned int _qp = 0;
  const Elem * const & _current_elem;

  /// The physical location of the element's quadrature Points, indexed by _qp
  const MooseArray<Point> & _q_point;
};
