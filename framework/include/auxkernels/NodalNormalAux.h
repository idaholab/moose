//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALAUX_H
#define NODALNORMALAUX_H

#include "AuxKernel.h"

class NodalNormalAux;
class NodalNormalsUserObject;

template <>
InputParameters validParams<NodalNormalAux>();

/**
 * Retreive a component of a nodal normal from a NodalNormalsUserObject.
 *
 * This is used for visualization purposes.
 * @see NodalNormalsUserObject
 */
class NodalNormalAux : public AuxKernel
{
public:
  NodalNormalAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The component of the normal as an enum
  MooseEnum _component_enum;
  /// The component of the normal as an integer
  unsigned int _component;
  /// User object holding the nodal normals
  const NodalNormalsUserObject & _nodal_normals;
};

#endif // NODALNORMALAUX_H
