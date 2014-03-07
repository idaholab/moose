#ifndef ADDSLAVEFLUXVECTORACTION_H
#define ADDSLAVEFLUXVECTORACTION_H

#include "MooseObjectAction.h"

#include <string>

class AddSlaveFluxVectorAction : public Action
{
public:
  AddSlaveFluxVectorAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddSlaveFluxVectorAction>();

#endif // ADDSLAVEFLUXVECTORACTION_H
