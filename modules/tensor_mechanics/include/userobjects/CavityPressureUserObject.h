/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAVITYPRESSUREUSEROBJECT_H
#define CAVITYPRESSUREUSEROBJECT_H

#include "GeneralUserObject.h"

class CavityPressureUserObject : public GeneralUserObject
{
public:
  CavityPressureUserObject(const InputParameters & parameters);

  virtual ~CavityPressureUserObject() {}

  virtual void initialSetup() {}

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void execute();

  virtual void initialize();
  virtual void finalize() {}

  Real getValue(const std::string & quantity) const;

protected:
  Real & _cavity_pressure;
  Real & _n0; // The initial number of moles of gas.

  const Real _initial_pressure;

  std::vector<const PostprocessorValue *> _material_input;

  const Real _R;

  const Real & _temperature;
  const bool _init_temp_given;
  const Real _init_temp;

  const Real & _volume;

  Real _start_time;
  const Real _startup_time;

  bool & _initialized;
};

template <>
InputParameters validParams<CavityPressureUserObject>();

#endif // CAVITYRESSUREPOSTPROCESSOR_H
