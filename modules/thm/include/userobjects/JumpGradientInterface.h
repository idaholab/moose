#pragma once

#include "InternalSideUserObject.h"

class JumpGradientInterface;

template <>
InputParameters validParams<JumpGradientInterface>();

/**
 * This class computes the jump of the gradient of a given quantity when using CONTINUOUS finite
 * element.
 * This class acts on the internal sides of the cell.
 */
class JumpGradientInterface : public InternalSideUserObject
{
public:
  JumpGradientInterface(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void destroy();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  Real getValue() const { return _value; }

protected:
  // Auxiliary system variable:
  AuxiliarySystem & _aux;
  // Gradient value:
  const VariableGradient & _grad_u;
  const VariableGradient & _grad_u_neighbor;
  // Name of the variable storing the jump:
  std::string _jump_name;
  // Number of the jump variable
  unsigned int _jump_number;
  // Temporary variable:
  Real _value;
};
