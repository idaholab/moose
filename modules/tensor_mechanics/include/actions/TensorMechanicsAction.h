#ifndef TENSORMECHANICSACTION_H
#define TENSORMECHANICSACTION_H

#include "Action.h"

class TensorMechanicsAction;

template<>
InputParameters validParams<TensorMechanicsAction>();

class TensorMechanicsAction : public Action
{
public:
  TensorMechanicsAction(const std::string & name, InputParameters params);

  virtual void act();

private:

};

#endif //TENSORMECHANICSACTION_H
