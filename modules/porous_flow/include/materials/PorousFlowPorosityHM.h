//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYHM_H
#define POROUSFLOWPOROSITYHM_H

#include "PorousFlowPorosity.h"

// Forward Declarations
class PorousFlowPorosityHM;

template <>
InputParameters validParams<PorousFlowPorosityHM>();

/**
 * Material designed to provide the porosity in hydro-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain +
 * (biot-1)*(effective_porepressure-reference_pressure)/solid_bulk)
 */
class PorousFlowPorosityHM : public PorousFlowPorosity
{
public:
  PorousFlowPorosityHM(const InputParameters & parameters);
};

#endif // POROUSFLOWPOROSITYHM_H
