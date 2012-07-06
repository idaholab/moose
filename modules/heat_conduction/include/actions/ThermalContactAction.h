#ifndef THERMALCONTACTACTION_H
#define THERMALCONTACTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class ThermalContactAction;

template<>
InputParameters validParams<ThermalContactAction>();

class ThermalContactAction : public Action
{
public:
  ThermalContactAction(const std::string & name, InputParameters params);

  virtual void act();

protected:
  void addBcs();
  void addAuxVariables();
  void addAuxBcs();
  void addMaterials();
  void addDiracKernels();
  void addVectors();
};

#endif // THERMALCONTACTACTION_H
