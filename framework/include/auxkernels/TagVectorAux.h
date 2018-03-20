//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TAGVECTORAUX_H
#define TAGVECTORAUX_H

#include "AuxKernel.h"

// Forward Declarations
class TagVectorAux;

template <>
InputParameters validParams<TagVectorAux>();

/**
 * Constant auxiliary value
 */
class TagVectorAux : public AuxKernel
{
public:
  TagVectorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  TagID _tag_id;
  const VariableValue & _v;
};

#endif // TAGVECTORAUX_H
