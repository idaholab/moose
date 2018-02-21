//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWPOROSITYTM_H
#define POROUSFLOWPOROSITYTM_H

#include "PorousFlowPorosity.h"

// Forward Declarations
class PorousFlowPorosityTM;

template <>
InputParameters validParams<PorousFlowPorosityTM>();

/**
 * Material designed to provide the porosity in thermo-mechanical simulations
 * biot + (phi0 - biot)*exp(-vol_strain + thermal_exp_coeff * (temperature - reference_temperature))
 */
class PorousFlowPorosityTM : public PorousFlowPorosity
{
public:
  PorousFlowPorosityTM(const InputParameters & parameters);
};

#endif // POROUSFLOWPOROSITYTM_H
