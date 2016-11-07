/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
#include "ZeroInterface.h"

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
                          public RandomInterface,
                          public ZeroInterface
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
