#ifndef THERMALCONTACTACTION_H
#define THERMALCONTACTACTION_H

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
  std::string getGapValueName() const;
  void addBcs();
  void addAuxVariables();
  void addAuxBcs();
  void addMaterials();
  void addDiracKernels();
  void addVectors();

  const std::string _penetration_var_name;
  const std::string _qpoint_penetration_var_name;
};

#endif // THERMALCONTACTACTION_H
