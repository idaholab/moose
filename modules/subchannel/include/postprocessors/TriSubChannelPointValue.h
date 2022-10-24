#pragma once

#include "GeneralPostprocessor.h"
#include "TriSubChannelMesh.h"

/**
 * Calculates a user selected variable at a user selected point in the assembly
 */
class TriSubChannelPointValue : public GeneralPostprocessor
{
public:
  TriSubChannelPointValue(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  TriSubChannelMesh & _mesh;
  const VariableName & _variable;
  const Real & _height;
  const int & _i_ch;
  Point _point;
  const unsigned int _var_number;
  const System & _system;
  Real _value;

public:
  static InputParameters validParams();
};
