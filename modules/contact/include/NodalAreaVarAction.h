#ifndef NODALAREAVARACTION_H
#define NODALAREAVARACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class NodalAreaVarAction: public Action
{
public:
  NodalAreaVarAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<NodalAreaVarAction>();

#endif
