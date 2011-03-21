#ifndef COPYNODALVARIABLESACTION_H
#define COPYNODALVARIABLESACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class CopyNodalVariablesAction : public Action
{
public:
  CopyNodalVariablesAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CopyNodalVariablesAction>();

#endif // COPYNODALVARIABLESACTION_H
