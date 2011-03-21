#ifndef ADDICACTION_H
#define ADDICACTION_H

#include "MooseObjectAction.h"

class AddICAction : public MooseObjectAction
{
public:
  AddICAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddICAction>();

#endif // ADDICACTION_H
