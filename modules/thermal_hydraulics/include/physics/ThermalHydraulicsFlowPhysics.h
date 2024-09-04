//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"
#include "InputParameters.h"
#include "NamingInterface.h"
#include "THMEnums.h"
#include "libmesh/fe_type.h"

class THMProblem;
class Factory;
class ThermalHydraulicsApp;
class FluidProperties;
class FlowChannelBase;

#define registerTHMFlowModelPhysicsBaseTasks(app_name, derived_name)                               \
  registerPhysicsBaseTasks(app_name, derived_name);                                                \
  registerMooseAction(app_name, derived_name, "THMPhysics:add_ic");                                \
  registerMooseAction(app_name, derived_name, "add_variable");                                     \
  registerMooseAction(app_name, derived_name, "add_material")

/**
 * Provides functions to setup the flow model.  Should be used by components that has flow in them
 */
class ThermalHydraulicsFlowPhysics : public virtual PhysicsBase, public NamingInterface
{
public:
  ThermalHydraulicsFlowPhysics(const InputParameters & params);

  static InputParameters validParams();

  /// Add a flow channel
  void addFlowChannel(const FlowChannelBase * c_ptr);

  // TODO: add here and implement all types needed
  enum InletTypeEnum
  {
    MdotTemperature
  };

  /**
   * Add an inlet boundary
   * @param boundary_component the name of the flow boundary component
   * @param inlet_type the type of inlet
   */
  void setInlet(const std::string & boundary_component, const InletTypeEnum & inlet_type);

  // TODO: add here and implement all types needed
  enum OutletTypeEnum
  {
    FixedPressure
  };

  /**
   * Add an outlet boundary
   * @param boundary_component the name of the flow boundary component
   * @param outlet_type the type of outlet
   */
  void setOutlet(const std::string & boundary_component, const OutletTypeEnum & outlet_type);

protected:
  virtual void initializePhysicsAdditional() override;

  /// Get the name of a function for a given parameter of the FlowModelPhysics
  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /**
   * Adds variables common to any flow model physics (A, P_hf, ...)
   * Note: we could move these to flow components since they pertain to the geometry
   */
  virtual void addCommonVariables();

  /**
   * Adds initial conditions common to any flow model physics
   */
  virtual void addCommonInitialConditions();

  /**
   * Adds common materials useful for any flow model physics
   */
  virtual void addCommonMaterials();

  /// The THM problem
  THMProblem * _sim;

  /// The flow channel component that built this class
  std::vector<const FlowChannelBase *> _flow_channels;

  /// The name of the user object that defines fluid properties
  const UserObjectName _fp_name;

  /// The name of the flow components
  std::vector<std::string> _component_names;
  /// The name of the inlet components
  std::vector<std::string> _inlet_components;
  /// The types of the inlets
  std::vector<InletTypeEnum> _inlet_types;
  /// The name of the outlet components
  std::vector<std::string> _outlet_components;
  /// The types of the outlets
  std::vector<OutletTypeEnum> _outlet_types;

  /// Gravitational acceleration vector
  RealVectorValue _gravity_vector;
  /// Gravitational acceleration magnitude
  Real _gravity_magnitude;

  /// The type of FE used for flow
  FEType _fe_type;
  /// Lump the mass matrix
  bool _lump_mass_matrix;

  // Names of variables for which derivative material properties need to be created
  std::vector<VariableName> _derivative_vars;

private:
  /// Create the objects for the inlet boundary conditions
  virtual void addInletBoundaries() = 0;
  /// Create the objects for the outlet boundary conditions
  virtual void addOutletBoundaries() = 0;

public:
  static const std::string AREA;
  static const std::string AREA_LINEAR;
  static const std::string HEAT_FLUX_WALL;
  static const std::string HEAT_FLUX_PERIMETER;
  static const std::string NUSSELT_NUMBER;
  static const std::string SURFACE_TENSION;
  static const std::string TEMPERATURE_WALL;
  static const std::string UNITY;
  static const std::string DIRECTION;
};
