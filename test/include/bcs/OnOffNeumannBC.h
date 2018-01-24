//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ONOFFNEUMANNBC_H
#define ONOFFNEUMANNBC_H

#include "NeumannBC.h"

class OnOffNeumannBC;

template <>
InputParameters validParams<OnOffNeumannBC>();

/**
 * NeumanBC with ability to turn on and off
 */
class OnOffNeumannBC : public NeumannBC
{
public:
  OnOffNeumannBC(const InputParameters & parameters);

  virtual bool shouldApply();

protected:
};

#endif /* ONOFFNEUMANNBC_H */
