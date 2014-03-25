#ifndef CAVITYPRESSUREPPACTION_H
#define CAVITYPRESSUREPPACTION_H

#include "Action.h"
#include "MooseTypes.h"

class CavityPressurePPAction: public Action
{
public:
  CavityPressurePPAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<CavityPressurePPAction>();


#endif // CAVITYPRESSUREPPACTION_H
