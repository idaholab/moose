#ifndef HEATCONDUCTIONMODEL_H
#define HEATCONDUCTIONMODEL_H

class Simulation;
class MooseApp;
class Factory;
class HeatStructure;

#include "InputParameters.h"
#include "libmesh/fe_type.h"

/**
 * Provides functions to setup the heat conduction model.
 *
 * This is a proxy class for the MOOSE Modules' heat conduction model
 */
class HeatConductionModel
{
public:
  HeatConductionModel(const std::string & name, const InputParameters & params);

  /**
   * Add flow-related variables
   * @param subdomain_id Block where the flow variables are defined
   */
  void addVariables(InputParameters & pars);
  void addMaterials(InputParameters & pars);
  void addHeatEquation(InputParameters & pars);

  /**
   * Get the FE type used for heat conduction
   * @return The finite element type
   */
  static const FEType & feType() { return _fe_type; }

private:
  /// The MOOSE application this is associated with
  MooseApp & _hc_app;

  /// The Factory associated with the MooseApp
  Factory & _hc_factory;

  Simulation & _hc_sim;

protected:
  /// Name of the component
  std::string _comp_name;

public:
  // variable names
  static const std::string DENSITY;
  static const std::string TEMPERATURE;
  static const std::string THERMAL_CONDUCTIVITY;
  static const std::string SPECIFIC_HEAT_CONSTANT_PRESSURE;

protected:
  // FE type used for heat conduction
  static FEType _fe_type;

  friend class GlobalSimParamAction;
};

#endif /* HEATCONDUCTIONMODEL_H */
