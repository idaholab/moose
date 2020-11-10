//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeochemistryReactorBase.h"
#include "AuxKernel.h"

/**
 * AuxKernel to extract information from a Geochemistry*Reactor to record into an AuxVariable
 */
class GeochemistryQuantityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GeochemistryQuantityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The species of interest
  const std::string & _species;

  const GeochemistryReactorBase & _reactor;

  const enum class QuantityChoiceEnum {
    MOLAL,
    MG_PER_KG,
    FREE_MG,
    FREE_CM3,
    NEGLOG10A,
    ACTIVITY,
    BULK_MOLES,
    SURFACE_CHARGE,
    SURFACE_POTENTIAL,
    TEMPERATURE,
    KINETIC_MOLES,
    KINETIC_ADDITIONS,
    MOLES_DUMPED,
    TRANSPORTED_MOLES_IN_ORIGINAL_BASIS
  } _quantity_choice;

  /**
   * index into mgd.surface_sorption_name corresponding to the species: this is used if quantity is
   * surface_charge or surface_potential
   */
  unsigned _surface_sorption_mineral_index;
};
