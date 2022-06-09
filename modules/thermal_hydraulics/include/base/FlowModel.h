//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "InputParameters.h"
#include "NamingInterface.h"
#include "THMEnums.h"
#include "libmesh/fe_type.h"

class THMProblem;
class Factory;
class ThermalHydraulicsApp;
class FluidProperties;
class FlowChannelBase;

/**
 * Provides functions to setup the flow model.  Should be used by components that has flow in them
 */
class FlowModel : public MooseObject, public NamingInterface
{
public:
  FlowModel(const InputParameters & params);

  /**
   * Gets a vector of the solution variables
   */
  std::vector<VariableName> getSolutionVariables() const { return _solution_vars; }

  /**
   * Initialize the model
   */
  virtual void init() = 0;

  /**
   * Add variables the model uses
   *
   */
  virtual void addVariables() = 0;

  /**
   * Add initial conditions
   */
  virtual void addInitialConditions() = 0;

  /**
   * Add MOOSE objects this model uses
   */
  virtual void addMooseObjects() = 0;

protected:
  THMProblem & _sim;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// The flow channel component that built this class
  FlowChannelBase & _flow_channel;

  /// The type of FE used for flow
  const FEType & _fe_type;

  /// The name of the user object that defines fluid properties
  const UserObjectName _fp_name;

  /// The component name
  const std::string _comp_name;

  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;
  /// Gravitational acceleration magnitude
  const Real _gravity_magnitude;

  /// Lump the mass matrix
  const bool _lump_mass_matrix;

  // Solution variable names
  std::vector<VariableName> _solution_vars;

  // Names of variables for which derivative material properties need to be created
  std::vector<VariableName> _derivative_vars;

  /// True if we output velocity as a vector-value field, false for outputting velocity as a scalar
  const bool & _output_vector_velocity;

  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /**
   * Adds variables common to any flow model (A, P_hf, ...)
   */
  virtual void addCommonVariables();

  /**
   * Adds initial conditions common to any flow model
   */
  virtual void addCommonInitialConditions();

  /**
   * Adds common MOOSE objects
   */
  virtual void addCommonMooseObjects();

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

  static InputParameters validParams();
};
