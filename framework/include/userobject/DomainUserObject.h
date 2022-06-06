//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "UserObject.h"
#include "BlockRestrictable.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"
#include "MooseError.h"

namespace libMesh
{
class Elem;
class QBase;
}

class DomainUserObject : public UserObject,
                         public BlockRestrictable,
                         public TwoMaterialPropertyInterface,
                         public NeighborCoupleable,
                         public MooseVariableDependencyInterface,
                         public TransientInterface,
                         public RandomInterface,
                         public ElementIDInterface
{
public:
  static InputParameters validParams();

  DomainUserObject(const InputParameters & parameters);

  void execute() override;

  /**
   * execute method that is called during ComputeUserObjects::onElement
   */
  virtual void executeOnElement() {}

  /**
   * execute method that is called during ComputeUserObjects::onBoundary
   */
  virtual void executeOnBoundary() {}

  /**
   * execute method that is called during ComputeUserObjects::onInternalSide
   */
  virtual void executeOnInternalSide() {}

  /**
   * execute method that is called during ComputeUserObjects::onInterface
   */
  virtual void executeOnInterface() {}

protected:
  /// the Moose mesh
  MooseMesh & _mesh;

  /// The current element pointer (available during execute())
  const Elem * const & _current_elem;

  /// The current element volume (available during execute())
  const Real & _current_elem_volume;

  /// current side of the current element
  const unsigned int & _current_side;

  const Elem * const & _current_side_elem;
  const Real & _current_side_volume;

  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// the neighboring element's volume
  const Real & _current_neighbor_volume;

  /// The quadrature points in physical space used in the element interior
  const MooseArray<Point> & _q_point;
  /// The quadrature rule used in the element interior
  const QBase * const & _qrule;
  /// The elemental Jacobian times quadrature weights in the element interior
  const MooseArray<Real> & _JxW;

  /// The quadrature points in physical space used on the element side/face
  const MooseArray<Point> & _q_point_face;
  /// The quadrature rule used on the element side/face
  const QBase * const & _qrule_face;
  /// The side element Jacobian times quadrature weights on the element side/face
  const MooseArray<Real> & _JxW_face;

  /// An array representing coordinate system volume modifications. Unity for Cartesian, 2piR for
  /// RZ, 4piR^2 for spherical
  const MooseArray<Real> & _coord;
};

inline void
DomainUserObject::execute()
{
  mooseError("Users of DomainUserObjects should call "
             "executeOnElement/executeOnBoundary/executeOnInternalSide/executeOnInterface");
}
