/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDPRIMARYSPECIESACTION_H
#define ADDPRIMARYSPECIESACTION_H

#include "Action.h"

class AddPrimarySpeciesAction;

template<>
InputParameters validParams<AddPrimarySpeciesAction>();


class AddPrimarySpeciesAction : public Action
{
public:
  AddPrimarySpeciesAction(const std::string & name, InputParameters params);

  virtual void act();

};

#endif // ADDPRIMARYSPECIESACTION_H
