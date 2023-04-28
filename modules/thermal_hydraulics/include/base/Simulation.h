//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalHydraulicsApp.h"
#include "FlowModel.h"
#include "Logger.h"
#include "ControlData.h"
#include "LoggingInterface.h"
#include "NamingInterface.h"
#include "libmesh/parallel_object.h"

class ActionWarehouse;
class Component;
class ClosuresBase;
class THMMesh;
class THMProblem;

/**
 * Main class for simulation (the driver of the simulation)
 */
class Simulation : public libMesh::ParallelObject, public LoggingInterface, public NamingInterface
{
public:
  Simulation(FEProblemBase & fe_problem, const InputParameters & params);
  virtual ~Simulation();

  /**
   * Gets the FE type for the flow in this simulation
   */
  const FEType & getFlowFEType() const { return _flow_fe_type; }

  /**
   * Sets up quadrature rules
   */
  virtual void setupQuadrature();

  /**
   * Initialize this simulation
   */
  virtual void initSimulation();

  /**
   * Initialize this simulation's components
   */
  virtual void initComponents();

  /**
   * Identifies the component loops
   */
  void identifyLoops();

  /**
   * Prints the component loops
   */
  void printComponentLoops() const;

  /**
   * Run the simulation
   */
  virtual void run();

  /**
   * Add a component into this simulation
   * @param type Type (the registered class name) of the component
   * @param name Name of the component
   * @param params Input parameters
   */
  virtual void
  addComponent(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Find out if simulation has a component with the given name
   * @param name The name of the component
   * @return true if the components exists, otherwise false
   */
  bool hasComponent(const std::string & name) const;

  /**
   * Find out if simulation has a component with the given name and specified type
   * @tparam T the type of the component we are requesting
   * @param name The name of the component
   * @return true if the component exists and has specified type, otherwise false
   */
  template <typename T>
  bool hasComponentOfType(const std::string & name) const;

  /**
   * Get component by its name
   * @tparam T the type of the component we are requesting
   * @param name The name of the component
   * @return Pointer to the component if found, otherwise throws and error
   */
  template <typename T>
  const T & getComponentByName(const std::string & name) const;

  /**
   * Return list of components available in the simulation
   */
  const std::vector<std::shared_ptr<Component>> & getComponents() { return _components; }

  /**
   * Add a closures object into this simulation
   *
   * @param[in] type   Closures class name
   * @param[in] name   Closures object name
   * @param[in] params   Input parameters
   */
  virtual void
  addClosures(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Return whether the simulation has a closures object
   *
   * @param[in] name   Closures object name
   */
  bool hasClosures(const std::string & name) const;

  /**
   * Get a pointer to a closures object
   *
   * @param[in] name   Closures object name
   */
  std::shared_ptr<ClosuresBase> getClosures(const std::string & name) const;

  /**
   * Queues a variable of type MooseVariableScalar to be added to the nonlinear or aux system.
   *
   * @param[in] nl   True if this is a nonlinear (solution) variable
   * @param[in] name   Name of the variable
   * @param[in] fe_type   FEType of the variable
   * @param[in] scaling_factor   Scaling factor for the variable
   */
  void
  addSimVariable(bool nl, const VariableName & name, FEType fe_type, Real scaling_factor = 1.0);

  /**
   * Queues a variable of type MooseVariable to be added to the nonlinear or aux system.
   *
   * @param[in] nl   True if this is a nonlinear (solution) variable
   * @param[in] name   Name of the variable
   * @param[in] fe_type   FEType of the variable
   * @param[in] subdomain_names   List of subdomain names to add the variable to
   * @param[in] scaling_factor   Scaling factor for the variable
   */
  void addSimVariable(bool nl,
                      const VariableName & name,
                      FEType fe_type,
                      const std::vector<SubdomainName> & subdomain_names,
                      Real scaling_factor = 1.0);

  /**
   * Queues a generic variable to be added to the nonlinear or aux system.
   *
   * @param[in] nl   True if this is a nonlinear (solution) variable
   * @param[in] var_type   Type (class) of the variable
   * @param[in] name   Name of the variable
   * @param[in] params   Input parameters for the variable
   */
  void addSimVariable(bool nl,
                      const std::string & var_type,
                      const VariableName & name,
                      const InputParameters & params);

  /**
   * Reports an error if the variable name is too long
   */
  void checkVariableNameLength(const std::string & name) const;

  void addConstantIC(const VariableName & var_name,
                     Real value,
                     const std::vector<SubdomainName> & block_names);
  void addFunctionIC(const VariableName & var_name,
                     const std::string & func_name,
                     const std::vector<SubdomainName> & block_names);
  void addConstantScalarIC(const VariableName & var_name, Real value);
  void addComponentScalarIC(const VariableName & var_name, const std::vector<Real> & value);

  void addSimInitialCondition(const std::string & type,
                              const std::string & name,
                              InputParameters params);

  /**
   * Add a control
   * @param type Type (registered name) of the control
   * @param name Name of the control
   * @param params Input parameters
   */
  void addControl(const std::string & type, const std::string & name, InputParameters params);

  void addFileOutputter(const std::string & name);
  void addScreenOutputter(const std::string & name);

  /**
   * Gets the vector of output names corresponding to a 1-word key string
   *
   * @param[in] key  string key that corresponds to an output names vector
   * @returns output names vector corresponding to key
   */
  std::vector<OutputName> getOutputsVector(const std::string & key) const;

  /**
   * Create mesh for this simulation
   */
  virtual void buildMesh();

  /**
   * Add variables involved in this simulation
   */
  virtual void addVariables();

  /**
   * Add component MOOSE objects
   */
  virtual void addMooseObjects();

  /**
   * Perform mesh setup actions such as setting up the coordinate system(s) and
   * creating ghosted elements.
   */
  virtual void setupMesh();

  /**
   * Get the ThermalHydraulicsApp
   */
  ThermalHydraulicsApp & getApp() { return _thm_app; }

  /**
   * Check the integrity of the simulation
   */
  virtual void integrityCheck() const;

  /**
   * Advance all of the state holding vectors / datastructures so that we can move to the next
   * timestep.
   */
  virtual void advanceState();

  /**
   * Check the integrity of the control data
   */
  virtual void controlDataIntegrityCheck();

  /**
   * Check integrity of coupling matrix used by the preconditioner
   */
  virtual void couplingMatrixIntegrityCheck() const;

  /**
   * Query if control data with name 'name' exists
   *
   * @param name The unique name of the control data
   * @return true if control data 'name' exists, false otherwise
   */
  template <typename T>
  bool hasControlData(const std::string & name)
  {
    if (_control_data.find(name) == _control_data.end())
      return false;
    else
      return dynamic_cast<ControlData<T> *>(_control_data[name]) != NULL;
  }

  /**
   * Get control data of type T and name 'name', if it does not exist it will be created
   *
   * @param name The unique name of the control data
   * @return Pointer to the control data of type T
   */
  template <typename T>
  ControlData<T> * getControlData(const std::string & name)
  {
    ControlData<T> * data = nullptr;
    if (_control_data.find(name) == _control_data.end())
    {
      data = new ControlData<T>(_thm_app, name);
      _control_data[name] = data;
    }
    else
      data = dynamic_cast<ControlData<T> *>(_control_data[name]);

    return data;
  }

  /**
   * Declare control data of type T and name 'name', if it does not exist it will be created
   *
   * @param name The unique name of the control data
   * @return Pointer to the control data of type T
   */
  template <typename T>
  ControlData<T> * declareControlData(const std::string & name, THMControl * ctrl)
  {
    ControlData<T> * data = getControlData<T>(name);
    if (!data->getDeclared())
    {
      // Mark the data for error checking
      data->setDeclared();
      data->setControl(ctrl);
    }
    else
      logError("Trying to declare '", name, "', but it was already declared.");

    return data;
  }

  /**
   * Gets the flag indicating whether an implicit time integration scheme is being used
   */
  const bool & getImplicitTimeIntegrationFlag() { return _implicit_time_integration; }

  /**
   * Are initial conditions specified from a file
   *
   * @return true if initial conditions are specified from a file
   */
  bool hasInitialConditionsFromFile() const;

  Logger & log() { return _log; }

  /**
   * Enable Jacobian checking
   *
   * @param state True for Jacobian checking, otherwise false
   */
  void setCheckJacobian(bool state) { _check_jacobian = state; }

  /**
   * Hint how to augment sparsity pattern between two elements.
   *
   * The augmentation will be symmetric
   */
  virtual void augmentSparsity(const dof_id_type & elem_id1, const dof_id_type & elem_id2);

  /**
   * Is velocity output as vector-valued field
   *
   * @return true for vector-valued field, false for scalar
   */
  bool getVectorValuedVelocity() { return _output_vector_velocity; }

  /**
   * Set if velocity is being output as a vector-valued field
   */
  void setVectorValuedVelocity(bool vector_velocity) { _output_vector_velocity = vector_velocity; }

  /**
   * Add additional relationship managers to run the simulation
   */
  void addRelationshipManagers();

protected:
  /**
   * Variable information
   */
  struct VariableInfo
  {
    /// True if the variable is a nonlinear (solution) variable; otherwise, aux
    bool _nl;
    /// Type (class) of the variable
    std::string _var_type;
    /// Input parameters
    InputParameters _params;

    VariableInfo() : _params(emptyInputParameters()) {}
  };

  /// THM mesh
  THMMesh & _thm_mesh;

  /// Pointer to FEProblem representing this simulation
  FEProblemBase & _fe_problem;

  /// The application this is associated with
  ThermalHydraulicsApp & _thm_app;

  /// The Factory associated with the MooseApp
  Factory & _thm_factory;

  /// List of components in this simulation
  std::vector<std::shared_ptr<Component>> _components;
  /// Map of components by their names
  std::map<std::string, std::shared_ptr<Component>> _comp_by_name;
  /// Map of component name to component loop name
  std::map<std::string, std::string> _component_name_to_loop_name;
  /// Map of loop name to model type
  std::map<std::string, THM::FlowModelID> _loop_name_to_model_id;

  /// Map of closures by their names
  std::map<std::string, std::shared_ptr<ClosuresBase>> _closures_by_name;

  /// variables for this simulation (name and info about the var)
  std::map<VariableName, VariableInfo> _vars;

  struct ICInfo
  {
    std::string _type;
    InputParameters _params;

    ICInfo() : _params(emptyInputParameters()) {}
    ICInfo(const std::string & type, const InputParameters & params) : _type(type), _params(params)
    {
    }
  };
  std::map<std::string, ICInfo> _ics;

  /// "Global" of this simulation
  const InputParameters & _thm_pars;

  /// finite element type for the flow in the simulation
  FEType _flow_fe_type;

  /**
   * Setup equations to be solved in this simulation
   */
  void setupEquations();

  /**
   * Setup reading initial conditions from a specified file, see 'initial_from_file' and
   * 'initial_from_file_timestep' parameters
   */
  void setupInitialConditionsFromFile();

  void setupInitialConditionObjects();

  /**
   * Sets the coordinate system for each subdomain
   */
  void setupCoordinateSystem();

  /**
   * Setup ctirical heat flux table user object
   */
  void setupCriticalHeatFluxTable();

  std::vector<OutputName> _outputters_all;
  std::vector<OutputName> _outputters_file;
  std::vector<OutputName> _outputters_screen;

  /// Control data created in the control logic system
  std::map<std::string, ControlDataValue *> _control_data;

  /// true if using implicit time integration scheme
  bool _implicit_time_integration;

  Logger _log;

  /// True if checking jacobian
  bool _check_jacobian;

  /// Additional sparsity pattern that needs to be added into the Jacobian matrix
  std::map<dof_id_type, std::vector<dof_id_type>> _sparsity_elem_augmentation;

  /// Flag indicating if velocity is output as vector-valued field
  bool _output_vector_velocity;

public:
  Real _zero;
};

template <typename T>
bool
Simulation::hasComponentOfType(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  if (it != _comp_by_name.end())
    return dynamic_cast<T *>((it->second).get()) != nullptr;
  else
    return false;
}

template <typename T>
const T &
Simulation::getComponentByName(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  if (it != _comp_by_name.end())
    return *dynamic_cast<T *>((it->second).get());
  else
    mooseError("Component '",
               name,
               "' does not exist in the simulation. Use hasComponent or "
               "checkComponnetByName before calling getComponent.");
}
