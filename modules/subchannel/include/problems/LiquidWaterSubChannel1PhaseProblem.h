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

#include "SubChannel1PhaseProblem.h"
#include "QuadSubChannelMesh.h"

class LiquidWaterSubChannel1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid water coolant
 */
class LiquidWaterSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  LiquidWaterSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual double computeFrictionFactor(double Re) override;
  virtual double computeFrictionFactor(double Re, int i_ch) override;
  QuadSubChannelMesh & _subchannel_mesh;

public:
  static InputParameters validParams();
};
