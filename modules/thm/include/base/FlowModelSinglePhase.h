#ifndef FLOWMODELSINGLEPHASE_H
#define FLOWMODELSINGLEPHASE_H

#include "FlowModel.h"

/**
 * Sets up the single-phase flow model using Euler's equations
 */
class FlowModelSinglePhase : public FlowModel
{
public:
  FlowModelSinglePhase(const std::string & name, const InputParameters & params);

  virtual void init() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  void addRDGMooseObjects();

public:
  static const std::string DENSITY;
  static const std::string DRAG_COEFFICIENT;
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
