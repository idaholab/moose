/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "InterWrapper1PhaseProblem.h"

class LiquidWaterInterWrapper1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid water coolant
 */
class LiquidWaterInterWrapper1PhaseProblem : public InterWrapper1PhaseProblem
{
public:
  LiquidWaterInterWrapper1PhaseProblem(const InputParameters & params);

protected:
  virtual Real computeFrictionFactor(Real Re) override;

public:
  static InputParameters validParams();
};
