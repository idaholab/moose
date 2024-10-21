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
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"

namespace libMesh
{
class Elem;
class QBase;
}

class ElementUserObject : public UserObject,
                          public BlockRestrictable,
                          public MaterialPropertyInterface,
                          public CoupleableMooseVariableDependencyIntermediateInterface,
                          public TransientInterface,
                          public RandomInterface,
                          public ElementIDInterface
{
public:
  static InputParameters validParams();

  ElementUserObject(const InputParameters & parameters);

protected:
  MooseMesh & _mesh;

  /// The current element pointer (available during execute())
  const Elem * const & _current_elem;

  /// The current element volume (available during execute())
  const Real & _current_elem_volume;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
};
