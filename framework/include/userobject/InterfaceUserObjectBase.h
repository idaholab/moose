//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UserObject.h"
#include "BoundaryRestrictableRequired.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "ElementIDInterface.h"
#include "FaceInfo.h"

/**
 *  Base class for implementing interface user objects
 */
class InterfaceUserObjectBase : public UserObject,
                                public BoundaryRestrictableRequired,
                                public TwoMaterialPropertyInterface,
                                public NeighborCoupleable,
                                public MooseVariableDependencyInterface,
                                public TransientInterface,
                                public ElementIDInterface
{
public:
  static InputParameters validParams();

  InterfaceUserObjectBase(const InputParameters & parameters);

protected:
  /**
   * Execute method.
   */
  virtual void execute() override {}

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() override {}

  MooseMesh & _mesh;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _normals;

  /// current element
  const Elem * const & _current_elem;
  /// the volume of the current element
  const Real & _current_elem_volume;
  /// current side of the current element
  const unsigned int & _current_side;
  /// the side element
  const Elem * const & _current_side_elem;
  /// current side volume
  const Real & _current_side_volume;

  /// The neighboring element
  const Elem * const & _neighbor_elem;
  /// current neighbor volume
  const Real & _current_neighbor_volume;
};
