#ifndef PLENUMPRESSUREPPACTION_H
#define PLENUMPRESSUREPPACTION_H

#include "Action.h"
#include "MooseTypes.h"

class PlenumPressurePPAction: public Action
{
public:
  PlenumPressurePPAction(const std::string & name, InputParameters params);

  virtual void act();

};

template<>
InputParameters validParams<PlenumPressurePPAction>();


#endif // PLENUMPRESSUREPPACTION_H
