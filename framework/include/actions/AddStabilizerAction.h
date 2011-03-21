#ifndef ADDSTABILIZERACTION_H
#define ADDSTABILIZERACTION_H

#include "MooseObjectAction.h"

class AddStabilizerAction: public MooseObjectAction
{
public:
  AddStabilizerAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddStabilizerAction>();

#endif //ADDSTABILIZERACTION_H
