#ifndef PRESETVELOCITY_H
#define PRESETVELOCITY_H

#include "PresetNodalBC.h"


class PresetVelocity : public PresetNodalBC
{
public:
  PresetVelocity(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpValue();

  const VariableValue & _u_old;
  const Real _velocity;
  Function * const _function;
};

template<>
InputParameters validParams<PresetVelocity>();

#endif /* PRESETVELOCITY_H */
