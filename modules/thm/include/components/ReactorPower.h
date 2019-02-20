#ifndef REACTORPOWER_H
#define REACTORPOWER_H

#include "Component.h"

class ReactorPower;

template <>
InputParameters validParams<ReactorPower>();

/**
 * Base class for components that provide reactor power
 */
class ReactorPower : public Component
{
public:
  ReactorPower(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual const VariableName & getPowerVariableName() const { return _power_var_name; }

protected:
  /// The scalar variable holding the value of power
  const VariableName _power_var_name;
};

#endif /* REACTORPOWER_H */
