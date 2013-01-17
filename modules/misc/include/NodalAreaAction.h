#ifndef NODALAREAACTION_H
#define NODALAREAACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaAction: public Action
{
public:
  NodalAreaAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<NodalAreaAction>();

#endif
