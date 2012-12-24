#ifndef ADDCOUPLEDEQSPECIESKERNELSACTION_H
#define ADDCOUPLEDEQSPECIESKERNELSACTION_H

#include "Action.h"

class AddCoupledEqSpeciesKernelsAction;

template<>
InputParameters validParams<AddCoupledEqSpeciesKernelsAction>();


class AddCoupledEqSpeciesKernelsAction : public Action
{
public:
  AddCoupledEqSpeciesKernelsAction(const std::string & name, InputParameters params);

  virtual void act();

};

#endif // ADDCOUPLEDEQSPECIESKERNELSACTION_H
