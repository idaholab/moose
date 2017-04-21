/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALTWOPHASEKERNELACTION_H
#define POLYCRYSTALTWOPHASEKERNELACTION_H

#include "PolycrystalKernelAction.h"

class PolycrystalTwoPhaseKernelAction;

template <>
InputParameters validParams<PolycrystalTwoPhaseKernelAction>();

class PolycrystalTwoPhaseKernelAction : public PolycrystalKernelAction
{
public:
  PolycrystalTwoPhaseKernelAction(const InputParameters & params);

protected:
  /// number of phases (here is 2)
  const unsigned int _phase_num;
  /// number of grains/variants of second phase
  unsigned int _second_phase_op_num;
  /// Ratio of surface energy to GB energy, e.g., interphase energy
  Real _en_ratio;
  /// Ratio of second-phase to parent-phase GB energy
  Real _second_phase_en_ratio;
  /// Ratio of second-phase to parent-phase GB mobility
  Real _mob_ratio;
};

#endif // POLYCRYSTALTWOPHASEKERNELACTION_H
