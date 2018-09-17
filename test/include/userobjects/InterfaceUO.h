//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUO_H
#define INTERFACEUO_H

#include "InterfaceUserObject.h"

class InterfaceUO;

template <>
InputParameters validParams<InterfaceUO>();

/**
 *
 */
class InterfaceUO : public InterfaceUserObject
{
public:
  InterfaceUO(const InputParameters & parameters);
  virtual ~InterfaceUO();

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  Real getMeanMatProp() const { return _mean_mat_prop; }
  Real getMeanVarJump() const { return _mean_var_jump; }

protected:
  const VariableValue & _u;
  const VariableValue & _u_neighbor;

  Real _mean_mat_prop;
  Real _mean_var_jump;
  Real _total_volume;

  const MaterialProperty<Real> & _diffusivity_prop;
  const MaterialProperty<Real> & _neighbor_diffusivity_prop;
};

#endif // INTERFACEUO_H
