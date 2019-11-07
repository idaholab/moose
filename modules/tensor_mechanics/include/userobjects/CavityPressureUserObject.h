//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class CavityPressureUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  CavityPressureUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override {}

  virtual Real computeCavityVolume();

  Real getValue(const MooseEnum & quantity) const;

protected:
  enum CAVITY_PRESSURE_USEROBJECT_QUANTITY
  {
    INITIAL_MOLES,
    CAVITY_PRESSURE
  };

  Real & _cavity_pressure;

  /// Initial number of moles of gas.
  Real & _n0;

  const Real _initial_pressure;

  std::vector<const PostprocessorValue *> _material_input;
  std::vector<const PostprocessorValue *> _volume;

  const Real _R;

  const Real & _temperature;
  const bool _init_temp_given;
  const Real _init_temp;

  const Real _startup_time;

  bool & _initialized;
  Real _start_time;
};

template <>
InputParameters validParams<CavityPressureUserObject>();
