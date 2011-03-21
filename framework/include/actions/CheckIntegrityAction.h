#ifndef CHECKINTEGRITYACTION_H
#define CHECKINTEGRITYACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class CheckIntegrityAction : public Action
{
public:
  CheckIntegrityAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CheckIntegrityAction>();

#endif // CHECKINTEGRITYACTION_H
