#ifndef ADAPTIVITYACTION_H_
#define ADAPTIVITYACTION_H_

#include "Action.h"

class AdaptivityAction: public Action
{
public:
  AdaptivityAction(const std::string & name, InputParameters params);

  virtual void act();

  unsigned int getSteps();
};

template<>
InputParameters validParams<AdaptivityAction>();

#endif //ADAPTIVITYACTION_H_
