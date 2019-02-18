#ifndef FLOWMODEL_H
#define FLOWMODEL_H

#include "MooseObject.h"
#include "InputParameters.h"
#include "Enums.h"
#include "libmesh/fe_type.h"

class Simulation;
class Factory;
class THMApp;
class FluidProperties;
class PipeBase;
class FlowModel;

template <>
InputParameters validParams<FlowModel>();

/**
 * Provides functions to setup the flow model.  Should be used by components that has flow in them
 */
class FlowModel : public MooseObject
{
public:
  FlowModel(const InputParameters & params);

  /// type for the flow equation
  enum EEquationType
  {
    CONTINUITY = 0,
    MOMENTUM = 1,
    ENERGY = 2,
    VOIDFRACTION = 3,
  };
  /// map of flow equation type string to enum
  static const std::map<std::string, EEquationType> _flow_equation_type_to_enum;

  /**
   * Gets a MooseEnum for flow equation type
   *
   * @param[in] name   default value
   * @returns MooseEnum for flow equation type
   */
  static MooseEnum getFlowEquationType(const std::string & name = "");

  /// Spatial discretization type
  enum ESpatialDiscretizationType
  {
    CG, ///< Continuous Galerkin Finite Element Method
    rDG ///< Reconstructed Discontinuous Galerkin / Finite Volume Method
  };
  /// map of spatial discretization type string to enum
  static const std::map<std::string, ESpatialDiscretizationType> _spatial_disc_type_to_enum;

  /**
   * Gets a MooseEnum for spatial discretization type
   *
   * @param[in] name   default value
   * @returns MooseEnum for spatial discretization type
   */
  static MooseEnum getSpatialDiscretizationMooseEnum(const std::string & name = "");

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

  /**
   * Get the FE type used for flow
   * @return The finite element type
   */
  static const FEType & feType() { return _fe_type; }

  /**
   * Gets the spatial discretization type for flow
   */
  static const ESpatialDiscretizationType & getSpatialDiscretizationType()
  {
    return _spatial_discretization;
  }

protected:
  Simulation & _sim;

  /// The application this is associated with
  THMApp & _app;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// The Pipe component that built this class
  PipeBase & _pipe;

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

  /// Linear cross-sectional area variable name
  const AuxVariableName _A_linear_name;

  /// Slope reconstruction type for rDG
  const MooseEnum _rdg_slope_reconstruction;

  /// Numerical flux user object name
  const UserObjectName _numerical_flux_name;

  // Solution variable names
  std::vector<VariableName> _solution_vars;

  // Names of variables for which derivative material properties need to be created
  std::vector<VariableName> _derivative_vars;

  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /**
   * Adds variables common to any flow model (A, D_h, P_hf, ...)
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
  static const std::string HEAT_FLUX_WALL;
  static const std::string HEAT_FLUX_PERIMETER;
  static const std::string HYDRAULIC_DIAMETER;
  static const std::string NUSSELT_NUMBER;
  static const std::string SURFACE_TENSION;
  static const std::string TEMPERATURE_WALL;
  static const std::string UNITY;
  static const std::string DIRECTION;

protected:
  /// The type of FE used for flow
  static FEType _fe_type;
  /// Spatial discretization
  static ESpatialDiscretizationType _spatial_discretization;

  friend class GlobalSimParamAction;
};

#endif /* FLOWMODEL_H */
