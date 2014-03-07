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
