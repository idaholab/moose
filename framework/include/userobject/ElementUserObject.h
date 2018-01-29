//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTUSEROBJECT_H
#define ELEMENTUSEROBJECT_H

// MOOSE includes
#include "UserObject.h"
#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "UserObjectInterface.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "RandomInterface.h"

// Forward Declarations
class ElementUserObject;

namespace libMesh
{
class Elem;
class QBase;
}

template <>
InputParameters validParams<ElementUserObject>();

class ElementUserObject : public UserObject,
                          public BlockRestrictable,
                          public MaterialPropertyInterface,
                          public UserObjectInterface,
                          public Coupleable,
                          public MooseVariableDependencyInterface,
                          public TransientInterface,
                          protected PostprocessorInterface,
                          public RandomInterface
{
public:
  ElementUserObject(const InputParameters & parameters);

protected:
  MooseMesh & _mesh;

  /// The current element pointer (available during execute())
  const Elem *& _current_elem;

  /// The current element volume (available during execute())
  const Real & _current_elem_volume;

  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
};

#endif
