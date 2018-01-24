//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COEFREACTION_H
#define COEFREACTION_H

#include "Reaction.h"

// Forward Declarations
class CoefReaction;

template <>
InputParameters validParams<CoefReaction>();

class CoefReaction : public Reaction
{
public:
  CoefReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// input parameter multiplied by the reaction kernel
  const Real _coef;
};

#endif // COEFREACTION_H
