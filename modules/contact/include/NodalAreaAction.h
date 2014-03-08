#ifndef NODALAREAACTION_H
#define NODALAREAACTION_H

#include "MooseObjectAction.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaAction: public MooseObjectAction
{
public:
  NodalAreaAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<NodalAreaAction>();

#endif
