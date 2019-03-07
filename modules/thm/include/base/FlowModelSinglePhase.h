#ifndef FLOWMODELSINGLEPHASE_H
#define FLOWMODELSINGLEPHASE_H

#include "FlowModel.h"

class FlowModelSinglePhase;

template <>
InputParameters validParams<FlowModelSinglePhase>();

/**
 * Sets up the single-phase flow model using Euler's equations
 */
class FlowModelSinglePhase : public FlowModel
{
public:
  FlowModelSinglePhase(const InputParameters & params);

  virtual void init() override;
  virtual void addVariables() override;
  virtual void addInitialConditions() override;
  virtual void addMooseObjects() override;

protected:
  std::string _raw_suffix;
  std::string _suffix;

  VariableName _rhoA_name;
  VariableName _rhouA_name;
  VariableName _rhoEA_name;
  VariableName _density_name;
  VariableName _total_energy_density_name;
  VariableName _momentum_density_name;
  VariableName _velocity_name;
  VariableName _pressure_name;
  VariableName _specific_volume_name;
  VariableName _specific_internal_energy_name;
  VariableName _temperature_name;
  VariableName _specific_total_enthalpy_name;

  void addRDGMooseObjects();

public:
  static const std::string DENSITY;
  static const std::string FRICTION_FACTOR_DARCY;
  static const std::string DYNAMIC_VISCOSITY;
  static const std::string HEAT_TRANSFER_COEFFICIENT_WALL;
  static const std::string MOMENTUM_DENSITY;
  static const std::string PRESSURE;
  static const std::string RHOA;
  static const std::string RHOEA;
  static const std::string RHOUA;
  static const std::string SOUND_SPEED;
  static const std::string SPECIFIC_HEAT_CONSTANT_PRESSURE;
  static const std::string SPECIFIC_HEAT_CONSTANT_VOLUME;
  static const std::string SPECIFIC_INTERNAL_ENERGY;
  static const std::string SPECIFIC_TOTAL_ENTHALPY;
  static const std::string SPECIFIC_VOLUME;
  static const std::string TEMPERATURE;
  static const std::string THERMAL_CONDUCTIVITY;
  static const std::string TOTAL_ENERGY_DENSITY;
  static const std::string VELOCITY;
};

#endif /* FLOWMODELSINGLEPHASE_H */
