/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TIMESTEPMATERIAL_H
#define TIMESTEPMATERIAL_H

#include "Material.h"

class TimeStepMaterial;

template <>
InputParameters validParams<TimeStepMaterial>();

/**
 * Store current time, dt, and time step number in material properties.
 */
class TimeStepMaterial : public Material
{
public:
  TimeStepMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _prop_dt;
  MaterialProperty<Real> & _prop_time;
  MaterialProperty<Real> & _prop_time_step;
};

#endif // TIMESTEPMATERIAL_H
