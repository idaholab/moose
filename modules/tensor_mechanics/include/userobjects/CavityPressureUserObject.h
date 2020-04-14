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

  /// The pressure within the cavity.
  Real & _cavity_pressure;

  /// Initial number of moles of gas.
  Real & _n0;

  const Real _initial_pressure;

  /// Postprocessors containing additional material released to the cavity.
  std::vector<const PostprocessorValue *> _material_input;

  /// Postprocessors whose sum equal the meshed cavity volume.
  std::vector<const PostprocessorValue *> _volume;

  /// The ideal gas constant.
  const Real _R;

  /// Reference to a postprocessor that contains the cavity temperature.
  const Real & _temperature;

  /// Whether or not an initial temperature is given.
  const bool _init_temp_given;

  /// The initial temperature.
  const Real _init_temp;

  /// The total time to ramp up the pressure to its initial value.
  const Real _startup_time;

  bool & _initialized;

  /// Additional volume that communicates with the cavity volume but is not meshed.
  std::vector<const PostprocessorValue *> _additional_volumes;

  /// The temperature of the additional volume.
  std::vector<const PostprocessorValue *> _temperature_of_additional_volumes;

  /// The time at which the pressure is at its maximum values
  Real _start_time;
};
