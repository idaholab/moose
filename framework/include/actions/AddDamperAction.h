#ifndef ADDDAMPERACTION_H_
#define ADDDAMPERACTION_H_

#include "MooseObjectAction.h"

class AddDamperAction: public MooseObjectAction
{
public:
  AddDamperAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddDamperAction>();

#endif //ADDDAMPERACTION_H_
