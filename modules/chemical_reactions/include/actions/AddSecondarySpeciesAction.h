#ifndef ADDSECONDARYSPECIESACTION_H
#define ADDSECONDARYSPECIESACTION_H

#include "Action.h"

class AddSecondarySpeciesAction;

template<>
InputParameters validParams<AddSecondarySpeciesAction>();


class AddSecondarySpeciesAction : public Action
{
public:
  AddSecondarySpeciesAction(const std::string & name, InputParameters params);

  virtual void act();

};

#endif // ADDSECONDARYSPECIESACTION_H
