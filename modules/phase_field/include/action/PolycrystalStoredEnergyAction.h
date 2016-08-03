/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALSTOREDENERGYACTION_H
#define POLYCRYSTALSTOREDENERGYACTION_H
#include "Action.h"

/**
 * Action that sets up ACSEDGPoly Kernels that add the stored energy contribution to grain growth models
 * This allows such models to simulate recrystallization as well.
 */
class PolycrystalStoredEnergyAction: public Action
{
public:
  PolycrystalStoredEnergyAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of grains to create
  unsigned int _op_num;

  /// base name for the order parameter variables
  std::string _var_name_base;

  /// number of deformed grains
  unsigned int _ndef;
};

template<>
InputParameters validParams<PolycrystalStoredEnergyAction>();

#endif //POLYCRYSTALSTOREDENERGYACTION_H
