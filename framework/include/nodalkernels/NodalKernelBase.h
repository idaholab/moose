//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "ResidualObject.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseVariableInterface.h"

/**
 * Base class for creating new types of nodal kernels
 */
class NodalKernelBase : public ResidualObject,
                        public BlockRestrictable,
                        public BoundaryRestrictable,
                        public GeometricSearchInterface,
                        public CoupleableMooseVariableDependencyIntermediateInterface,
                        public MooseVariableInterface<Real>
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  NodalKernelBase(const InputParameters & parameters);

  /**
   * Gets the variable this is active on
   * @return the variable
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  /// Reference to FEProblemBase
  FEProblemBase & _fe_problem;

  /// variable this works on
  MooseVariable & _var;

  /// current node being processed
  const Node * const & _current_node;

  /// Quadrature point index
  unsigned int _qp;
};
