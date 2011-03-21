#ifndef ADDAUXBCACTION_H
#define ADDAUXBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddAuxBCAction : public Action
{
public:
  AddAuxBCAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddAuxBCAction>();

#endif // ADDAUXBCACTION_H
