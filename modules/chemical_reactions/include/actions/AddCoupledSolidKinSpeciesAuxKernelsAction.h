#ifndef ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H
#define ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H

#include "Action.h"

class AddCoupledSolidKinSpeciesAuxKernelsAction;

template<>
InputParameters validParams<AddCoupledSolidKinSpeciesAuxKernelsAction>();


class AddCoupledSolidKinSpeciesAuxKernelsAction : public Action
{
public:
  AddCoupledSolidKinSpeciesAuxKernelsAction(const std::string & name, InputParameters params);

  virtual void act();

};

#endif // ADDCOUPLEDSOLIDKINSPECIESAUXKERNELSACTION_H
