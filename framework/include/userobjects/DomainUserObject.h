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
#include "ThreeMaterialPropertyInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"
#include "MooseError.h"

class MooseVariableFieldBase;

namespace libMesh
{
class Elem;
class QBase;
}

/**
 * This user object allows related evaluations on elements, boundaries, internal sides,
 * interfaces in one single place. DomainUserObject is still block restrictable.
 * While evaluations on elements, boundaries and internal sides always happen,
 * a parameter 'interface_boundaries' needs to be set to invoke evaluations on interfaces.
 * We require this parameter for interface evaluations because we want to enforce sanity
 * checks on coupling variables that are not defined on the domain this user object works on
 * but only are available on the other side of the interfaces. All sides of an interface must
 * connect with the subdomain of DomainUserObject.
 * With this user object, evaluations that would have to be put into ElementUserObject,
 * SideUserObject, InternalSideUserObject, and InterfaceUserObject separately may be combined.
 */
class DomainUserObject : public UserObject,
                         public BlockRestrictable,
                         public ThreeMaterialPropertyInterface,
                         public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
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
   * execute method that is called during ComputeUserObjects::onExternalSide
   */
  virtual void executeOnExternalSide(const Elem * /*elem*/, unsigned int /*side*/) {}

  /**
   * execute method that is called during ComputeUserObjects::onInterface
   */
  virtual void executeOnInterface() {}

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

  /**
   * method that is called right before executeOnInterface; sets the data to face
   */
  void preExecuteOnInterface();

  /**
   * Return whether this object should run \p executeOnInterface
   */
  bool shouldExecuteOnInterface() const;

  void checkVariable(const MooseVariableFieldBase & variable) const override;

protected:
  const MooseArray<Point> & qPoints() const { return *_current_q_point; }
  const QBase & qRule() const { return *_current_q_rule; }
  const MooseArray<Real> & JxW() const { return *_current_JxW; }
  const MooseArray<Real> & coord() const { return _coord; }
  const MooseArray<Point> & normals() const { return _normals; }

  /**
   * Routes through to \p Coupleable::getFieldVar, but also inserts the return variable into a set
   * of field variables to check on interface-connected blocks, as opposed to our blocks, when
   * performing our block-restriction integrity check.
   * The argument \p interfaces is optional specifying on what interfaces the variable is expected
   * to be available, i.e. the field variable is defined over elements out side of the domain but
   * connecting the subdomain with the interfaces. Default value means that the variable should be
   * available on all interfaces.
   * Note: a field variable on interfaces is not required to be defined on the subdomain of this
   *       domain user object.
   */
  const MooseVariableFieldBase *
  getInterfaceFieldVar(const std::string & var_name,
                       unsigned int comp,
                       const std::set<BoundaryID> * interfaces = nullptr);

  /// the Moose mesh
  MooseMesh & _mesh;

  /// The current element pointer (available during all execute functions)
  const Elem * const & _current_elem;

  /// The current element volume (available during all execute functions)
  const Real & _current_elem_volume;

  /// Current side of the current element (available during executeOnInternalSide() and
  /// executeOnBoundary() and executeOnInterface())
  const unsigned int & _current_side;

  /// Current side of the current element (available during executeOnInternalSide() and
  /// executeOnBoundary() and executeOnInterface())
  const Elem * const & _current_side_elem;

  /// Current side volume (available during executeOnInternalSide() and executeOnBoundary() and
  /// executeOnInterface())
  const Real & _current_side_volume;

  /// The neighboring element (available during executeOnInternalSide() and executeOnInterface())
  const Elem * const & _neighbor_elem;

  /// the neighboring element's volume (available during executeOnInternalSide() and
  /// executeOnInterface())
  const Real & _current_neighbor_volume;

  /// The boundary ID (available during executeOnBoundary() and executeOnInterface())
  const BoundaryID & _current_boundary_id;

  /// The unit norm at quadrature points on the element side/face from the current element
  /// perpendicular to the side
  const MooseArray<Point> & _normals;

  /// The set of boundary IDs on which this object should perform \p executeOnInterface
  std::set<BoundaryID> _interface_bnd_ids;

  /// The set of blocks connected to our blocks through boundaries of the \p _interface_bnd_ids data member
  std::map<BoundaryID, std::set<SubdomainID>> _interface_connected_blocks;

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

  /// A map storing the set of boundaries where variables we wish to evaluate
  std::map<VariableName, std::set<BoundaryID>> _var_interfaces;
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
DomainUserObject::preExecuteOnInterface()
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

inline bool
DomainUserObject::shouldExecuteOnInterface() const
{
  return _interface_bnd_ids.count(_current_boundary_id);
}
