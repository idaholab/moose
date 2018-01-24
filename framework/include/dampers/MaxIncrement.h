//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MAXINCREMENT_H
#define MAXINCREMENT_H

// Moose Includes
#include "ElementDamper.h"

// Forward Declarations
class MaxIncrement;

template <>
InputParameters validParams<MaxIncrement>();

/**
 * TODO
 */
class MaxIncrement : public ElementDamper
{
public:
  MaxIncrement(const InputParameters & parameters);

protected:
  virtual Real computeQpDamping() override;

  ///The maximum Newton increment for the variable.
  Real _max_increment;
};

#endif // MAXINCREMENT_H
