//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
