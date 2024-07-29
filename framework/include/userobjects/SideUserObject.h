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
#include "BoundaryRestrictableRequired.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "ElementIDInterface.h"
#include "FaceInfo.h"

/**
 * Base class for user objects executed one or more sidesets, which may be
 * on the outer boundary of the mesh, or be internal to mesh, blocks etc
 */
class SideUserObject : public UserObject,
                       public BoundaryRestrictableRequired,
                       public MaterialPropertyInterface,
                       public CoupleableMooseVariableDependencyIntermediateInterface,
                       public TransientInterface,
                       public ElementIDInterface
{
public:
  static InputParameters validParams();

  SideUserObject(const InputParameters & parameters);

protected:
  MooseMesh & _mesh;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _normals;

  const Elem * const & _current_elem;
  /// current side of the current element
  const unsigned int & _current_side;

  const Elem * const & _current_side_elem;
  const Real & _current_side_volume;

  const BoundaryID & _current_boundary_id;

  /// Holds the FaceInfos to loop on to consider all active neighbors of an element on a given side
  std::vector<const FaceInfo *> _face_infos;

  /**
   * Computes the local FaceInfo(s) to use in functor arguments and interpolations.
   * Adaptivity/refinement may mean that an element with a given side has multiple active
   * faces (each a different FaceInfo) with its more refined neighbor.
   * Note that face info could hold the element from the other side of the
   * sideset. Sidesets are oriented!
   */
  void getFaceInfos();
};
