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

  void execute() override final;

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
   * method that is called right before executeOnElement; sets the data to volumetric
   */
  void preExecuteOnElement();

  /**
   * method that is called right before executeOnBoundary; sets the data to face
   */
  void preExecuteOnBoundary();

  /**
   * method that is called right before executeOnInternalSide; sets the data to face
   */
  void preExecuteOnInternalSide();

protected:
  const MooseArray<Point> & qPoints() const { return *_current_q_point; }
  const QBase & qRule() const { return *_current_q_rule; }
  const MooseArray<Real> & JxW() const { return *_current_JxW; }
  const MooseArray<Real> & coord() const { return _coord; }
  const MooseArray<Point> & normals() const { return _normals; }

  /// the Moose mesh
  MooseMesh & _mesh;

  /// The current element pointer (available during all execute functions)
  const Elem * const & _current_elem;

  /// The current element volume (available during all execute functions)
  const Real & _current_elem_volume;

  /// Current side of the current element (available during executeOnInternalSide() and executeOnBoundary())
  const unsigned int & _current_side;

  /// Current side of the current element (available during executeOnInternalSide() and executeOnBoundary())
  const Elem * const & _current_side_elem;

  /// Current side volume (available during executeOnInternalSide() and executeOnBoundary())
  const Real & _current_side_volume;

  /// The neighboring element (available during executeOnInternalSide())
  const Elem * const & _neighbor_elem;

  /// the neighboring element's volume (available during executeOnInternalSide())
  const Real & _current_neighbor_volume;

  /// The boundary ID (available during executeOnBoundary())
  const BoundaryID & _current_boundary_id;

private:
  void setVolumeData();

  void setFaceData();

  /// A pointer to the current volume/face quadrature points
  const MooseArray<Point> * _current_q_point;
  /// A pointer to the current volume/face quadrature rule
  const QBase * _current_q_rule;
  /// A pointer to the current JxW
  const MooseArray<Real> * _current_JxW;

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
  /// The unit norm at quadrature points on the element side/face from the current element perpendicular to the side
  const MooseArray<Point> & _normals;
};

inline void
DomainUserObject::execute()
{
  mooseError("Users of DomainUserObjects should call "
             "executeOnElement/executeOnBoundary/executeOnInternalSide");
}

inline void
DomainUserObject::preExecuteOnElement()
{
  setVolumeData();
}

inline void
DomainUserObject::preExecuteOnBoundary()
{
  setFaceData();
}

inline void
DomainUserObject::preExecuteOnInternalSide()
{
  setFaceData();
}

inline void
DomainUserObject::setVolumeData()
{
  _current_q_point = &_q_point;
  _current_q_rule = _qrule;
  _current_JxW = &_JxW;
}

inline void
DomainUserObject::setFaceData()
{
  _current_q_point = &_q_point_face;
  _current_q_rule = _qrule_face;
  _current_JxW = &_JxW_face;
}
