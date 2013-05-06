#ifndef NODALAREAAUXACTION_H
#define NODALAREAAUXACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaAuxAction;

template<>
InputParameters validParams<NodalAreaAuxAction>();

class NodalAreaAuxAction: public Action
{
public:
  NodalAreaAuxAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const BoundaryName _slave;
};


#endif // CONTACTACTION_H
