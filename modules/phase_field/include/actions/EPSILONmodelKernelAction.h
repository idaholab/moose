

#pragma once

#include "Action.h"


class EPSILONmodelKernelAction : public Action
{
public:
  static InputParameters validParams();

  EPSILONmodelKernelAction(const InputParameters & params);

  virtual void act();

protected:
  /// number of grains to create
  const unsigned int _op_num;

  /// base name for the order parameter variables
  const std::string _var_name_base;

};
