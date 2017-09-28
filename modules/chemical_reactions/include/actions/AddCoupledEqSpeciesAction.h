/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDCOUPLEDEQSPECIESACTION_H
#define ADDCOUPLEDEQSPECIESACTION_H

#include "Action.h"

class AddCoupledEqSpeciesAction;

template <>
InputParameters validParams<AddCoupledEqSpeciesAction>();

class AddCoupledEqSpeciesAction : public Action
{
public:
  AddCoupledEqSpeciesAction(const InputParameters & params);

  virtual void act() override;

protected:
  const std::string _reactions;
  const std::vector<NonlinearVariableName> _primary_species;
  const std::vector<AuxVariableName> _secondary_species;
  const std::vector<VariableName> _pressure_var;
  const RealVectorValue _gravity;
};

#endif // ADDCOUPLEDEQSPECIESACTION_H
