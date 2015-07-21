/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDSECONDARYSPECIESACTION_H
#define ADDSECONDARYSPECIESACTION_H

#include "Action.h"

class AddSecondarySpeciesAction;

template<>
InputParameters validParams<AddSecondarySpeciesAction>();


class AddSecondarySpeciesAction : public Action
{
public:
  AddSecondarySpeciesAction(const InputParameters & params);
  AddSecondarySpeciesAction(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

  virtual void act();

};

#endif // ADDSECONDARYSPECIESACTION_H
