//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYTHM_H
#define POROUSFLOWPOROSITYTHM_H

#include "PorousFlowPorosity.h"

// Forward Declarations
class PorousFlowPorosityTHM;

template <>
InputParameters validParams<PorousFlowPorosityTHM>();

/**
 * Material designed to provide the porosity in thermo-hydro-mechanical simulations
 * biot + (phi0 - biot) * exp(-vol_strain
 *    + coeff * (effective_pressure - reference_pressure)
 *    + thermal_exp_coeff * (temperature - reference_temperature))
 */
class PorousFlowPorosityTHM : public PorousFlowPorosity
{
public:
  PorousFlowPorosityTHM(const InputParameters & parameters);
};

#endif // POROUSFLOWPOROSITYTHM_H
