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
 * "Paints" the ID of of the physical "node" in the cluster the element
 * is located on.  Useful for examining partition schemes.
 */
class HardwareIDAux : public AuxKernel
{
public:
  static InputParameters validParams();

  HardwareIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const RankMap & _rank_map;
};
