#pragma once

#include "Action.h"

class GammaModelKernelAction : public Action
{
public:
  static InputParameters validParams();

  GammaModelKernelAction(const InputParameters & params);

  virtual void act() override;

protected:
  // Number of grains to create.
  const unsigned int _op_num;

  // Base name for the order parameter variables.
  const std::string _var_name_base;
};
