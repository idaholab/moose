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
#include "PolycrystalUserObjectBase.h"

// Forward Declarations

/**
 * Visualize the location of grain boundaries in a polycrystalline simulation.
 */
class VoronoiICAux : public AuxKernel
{
public:
  static InputParameters validParams();

  VoronoiICAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const PolycrystalUserObjectBase & _poly_ic_uo;

  std::vector<unsigned int> _grain_ids;
};
