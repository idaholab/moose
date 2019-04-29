#pragma once

#include "GeneralPostprocessor.h"
#include "MooseObjectParameterName.h"

class RealComponentParameterValuePostprocessor;

template <>
InputParameters validParams<RealComponentParameterValuePostprocessor>();

class RealComponentParameterValuePostprocessor : public GeneralPostprocessor
{
public:
  RealComponentParameterValuePostprocessor(const InputParameters & parameters);

  virtual void initialize();
  virtual Real getValue();
  virtual void execute();

protected:
  FEProblemBase & _fe_problem;
  InputParameterWarehouse & _input_parameter_warehouse;

  const std::string & _component_name;
  const std::string & _param_name;
  MooseObjectParameterName _ctrl_param_name;

  Real _value;
};
