//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MooseFunctorArguments.h"

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

  void setSubdomains(const std::set<SubdomainID> & sub_ids) { _sub_ids = &sub_ids; }

protected:
  Moose::NodeArg nodeArg() const { return Moose::NodeArg{_current_node, _sub_ids}; }

  /// Reference to FEProblemBase
  FEProblemBase & _fe_problem;

  /// variable this works on
  MooseVariable & _var;

  /// current node being processed
  const Node * const & _current_node;

  /// Quadrature point index
  unsigned int _qp;

  /// The set of subdomains connected to the current node
  const std::set<SubdomainID> * _sub_ids;
};
