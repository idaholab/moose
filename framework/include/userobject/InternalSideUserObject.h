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
#include "BlockRestrictable.h"
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "ElementIDInterface.h"

class InternalSideUserObject;

template <>
InputParameters validParams<InternalSideUserObject>();

/**
 *
 */
class InternalSideUserObject : public UserObject,
                               public BlockRestrictable,
                               public TwoMaterialPropertyInterface,
                               public NeighborCoupleable,
                               public MooseVariableDependencyInterface,
                               public TransientInterface,
                               public ElementIDInterface
{
public:
  static InputParameters validParams();

  InternalSideUserObject(const InputParameters & parameters);

protected:
  MooseMesh & _mesh;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _normals;

  /// pointer to the current element object
  const Elem * const & _current_elem;

  /// the volume of the current element
  const Real & _current_elem_volume;

  /// current side of the current element
  const unsigned int & _current_side;

  const Elem * const & _current_side_elem;
  const Real & _current_side_volume;

  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// the neighboring element's volume
  const Real & _current_neighbor_volume;

  /// The volume (or length) of the current neighbor
  const Real & getNeighborElemVolume();
};
