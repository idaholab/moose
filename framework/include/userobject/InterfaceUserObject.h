//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUSEROBJECT_H
#define INTERFACEUSEROBJECT_H

#include "UserObject.h"
#include "BoundaryRestrictableRequired.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"

class InterfaceUserObject;

template <>
InputParameters validParams<InterfaceUserObject>();

/**
 *
 */
class InterfaceUserObject : public UserObject,
                            public BoundaryRestrictableRequired,
                            public TwoMaterialPropertyInterface,
                            public NeighborCoupleable,
                            public MooseVariableDependencyInterface,
                            public UserObjectInterface,
                            public TransientInterface,
                            public PostprocessorInterface
{
public:
  InterfaceUserObject(const InputParameters & parameters);

protected:
  MooseMesh & _mesh;

  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _normals;

  const Elem *& _current_elem;
  /// current side of the current element
  unsigned int & _current_side;

  const Elem *& _current_side_elem;
  const Real & _current_side_volume;

  /// The neighboring element
  const Elem *& _neighbor_elem;

  /// The volume (or length) of the current neighbor
  const Real & getNeighborElemVolume();
};

#endif /* INTERFACEUSEROBJECT_H */
