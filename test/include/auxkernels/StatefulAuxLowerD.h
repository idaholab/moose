//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Coupled auxiliary value
 */
class StatefulAuxLowerD : public AuxKernel
{
public:
  static InputParameters validParams();

  StatefulAuxLowerD(const InputParameters & parameters);

  virtual ~StatefulAuxLowerD() {}

protected:
  virtual Real computeValue();

  // Lower dimensional variable from a Mortar constraint
  const MooseArray<Real> & _flux;
};
