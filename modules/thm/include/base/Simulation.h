#pragma once

#include "FEProblem.h"
#include "THMApp.h"
#include "FlowModel.h"
#include "ControlData.h"
#include "LoggingInterface.h"
#include "NamingInterface.h"

class ActionWarehouse;
class Component;
class THMMesh;

/**
 * Main class for simulation (the driver of the simulation)
 */
class Simulation : public LoggingInterface, public NamingInterface
{
public:
  Simulation(ActionWarehouse & action_warehouse);
  virtual ~Simulation();

  /**
   * Get the reference to the mesh
   * @return The reference to the mesh
   */
  THMMesh & mesh();

  /**
   * Get the input parameters for the simulation
   * @return The reference to the input parameters
   */
  InputParameters & params();

  /**
   * Get the FEProblem associated with this simulation
   * @return The reference to the FEProblem
   */
  FEProblem & feproblem();

  /**
   * Get parameter from simulation (global one)
   * @param name The name on the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParamTempl(const std::string & name) const;

  /**
   * Gets the FE type for the flow in this simulation
   */
  const FEType & getFlowFEType() const { return _flow_fe_type; }

  /**
   * Gets the spatial discretization type for flow
   */
  const FlowModel::ESpatialDiscretizationType & getSpatialDiscretization() const
  {
    return _spatial_discretization;
  }

  /**
   * Sets up quadrature rules
   */
  virtual void setupQuadrature();

  /**
   * Sets the simulation up
   */
  virtual void build();

  /**
   * Initialize this simulation
   */
  virtual void init();

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
   * Adds postprocessors for cumulative numbers of linear and nonlinear iterations
   */
  void addIterationCountPostprocessors();

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
  void addComponent(const std::string & type, const std::string & name, InputParameters params);

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
   * Called by a component to announce a variable
   * @param nl True is nonlinear variable is being added
   * @param name The name of the variable
   * @param type Type of the variable
   * @param subdomain_id Subdomain of the variable
   * @param scaling_factor Scaling factor for the variable
   */
  void addVariable(bool nl,
                   const std::string & name,
                   FEType type,
                   const SubdomainName & subdomain_name = "",
                   Real scaling_factor = 1.);
  void addVariable(bool nl,
                   const std::string & name,
                   FEType type,
                   const std::vector<SubdomainName> & subdomain_names,
                   Real scaling_factor = 1.);

  void addConstantIC(const std::string & var_name, Real value, const SubdomainName & block_name);
  void addConstantIC(const std::string & var_name,
                     Real value,
                     const std::vector<SubdomainName> & block_names);
  void addFunctionIC(const std::string & var_name,
                     const std::string & func_name,
                     const SubdomainName & block_name);
  void addFunctionIC(const std::string & var_name,
                     const std::string & func_name,
                     const std::vector<SubdomainName> & block_names);
  void addConstantScalarIC(const std::string & var_name, Real value);
  void addComponentScalarIC(const std::string & var_name, const std::vector<Real> & value);

  void
  addInitialCondition(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a kernel
   * @param type Type (registered name) of the kernel
   * @param name Name of the kernel
   * @param params Input parameters
   */
  void addKernel(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a DG kernel
   *
   * @param[in] type     Type (registered name) of the object
   * @param[in] name     Name of the object
   * @param[in] params   Input parameters
   */
  void addDGKernel(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds an auxiliary kernels
   * @param type Type (registered name) of the auxiliary kernel
   * @param name Name of the auxiliary kernel
   * @param params Input parameters
   */
  void addAuxKernel(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a scalar kernels
   * @param type Type (registered name) of the scalar kernel
   * @param name Name of the scalar kernel
   * @param params Input parameters
   */
  void addScalarKernel(const std::string & type, const std::string & name, InputParameters params);

  void
  addAuxScalarKernel(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a boundary condition
   * @param type Type (registered name) of the boundary condition
   * @param name Name of the boundary condition
   * @param params Input parameters
   */
  void
  addBoundaryCondition(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds an auxiliary boundary condition
   * @param type Type (registered name) of the auxiliary boundary condition
   * @param name Name of the auxiliary boundary condition
   * @param params Input parameters
   */
  void addAuxBoundaryCondition(const std::string & type,
                               const std::string & name,
                               InputParameters params);

  /**
   * Adds a function
   * @param type Type (registered name) of the function
   * @param name Name of the function
   * @param params Input parameters
   */
  void addFunction(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a Material
   * @param type Type (registered name) of the material
   * @param name Name of the material
   * @param params Input parameters
   */
  void addMaterial(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a postprocessor
   * @param type Type (registered name) of the prostprocessor
   * @param name Name of the postprocessor
   * @param params Input parameters
   */
  void addPostprocessor(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a vector postprocessor
   * @param type Type (registered name) of the prostprocessor
   * @param name Name of the postprocessor
   * @param params Input parameters
   */
  void addVectorPostprocessor(const std::string & type,
                              const std::string & name,
                              InputParameters params);

  /**
   * Adds a constraint
   * @param type Type (registered name) of the constraint
   * @param name Name of the constraint
   * @param params Input parameters
   */
  void addConstraint(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Adds a user object
   * @param type Type (registered name) of the user object
   * @param name Name of the user object
   * @param params Input parameters
   */
  void addUserObject(const std::string & type, const std::string & name, InputParameters params);

  /**
   * Add a transfer
   * @param type Type (registered name) of the transfer
   * @param name Name of the transfer
   * @param params Input parameters
   */
  void addTransfer(const std::string & type, const std::string & name, InputParameters params);

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
   * Get a reference to a postprocessor value
   * @param name Name of the postprocessor
   * @return The reference to the postprocessor value
   */
  Real & getPostprocessorValue(const std::string & name);

  bool hasFunction(const std::string & name, THREAD_ID tid = 0);

  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  bool hasUserObject(const UserObjectName & name) { return _fe_problem->hasUserObject(name); }

  template <class T>
  const T & getUserObjectTempl(const UserObjectName & name)
  {
    return _fe_problem->getUserObjectTempl<T>(name);
  }

  /**
   * Create mesh for this simulation
   */
  void buildMesh();

  /**
   * Add variables involved in this simulation
   */
  void addVariables();

  /**
   * Add components based physics
   */
  void addComponentPhysics();

  /**
   * Perform mesh setup actions such as setting up the coordinate system(s) and
   * creating ghosted elements.
   */
  void setupMesh();

  /**
   * Get the THMApp
   */
  THMApp & getApp() { return _app; }

  /**
   * Check the integrity of the simulation
   */
  virtual void integrityCheck() const;

  /**
   * Check the integrity of the control data
   */
  virtual void controlDataIntegrityCheck();

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
   * @param name The control object that declared this data
   * @return Pointer to the control data of type T
   */
  template <typename T>
  ControlData<T> * getControlData(const std::string & name)
  {
    ControlData<T> * data = nullptr;
    if (_control_data.find(name) == _control_data.end())
    {
      data = new ControlData<T>(name);
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
  bool hasInitialConditionsFromFile();

protected:
  struct VariableInfo
  {
    bool _nl; ///< true if the variable is non-linear
    FEType _type;
    std::set<SubdomainName> _subdomain;
    Real _scaling_factor;
  };
  ActionWarehouse & _action_warehouse;
  std::shared_ptr<THMMesh> _mesh;

  /// Pointer to FEProblem representing this simulation
  FEProblem * _fe_problem;

  /// The application this is associated with
  THMApp & _app;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Builds Actions
  ActionFactory & _action_factory;

  /// List of components in this simulation
  std::vector<std::shared_ptr<Component>> _components;
  /// Map of components by their names
  std::map<std::string, std::shared_ptr<Component>> _comp_by_name;
  /// Map of component name to component loop name
  std::map<std::string, std::string> _component_name_to_loop_name;
  /// Map of loop name to model type
  std::map<std::string, THM::FlowModelID> _loop_name_to_model_id;

  /// variables for this simulation (name and info about the var)
  std::map<std::string, VariableInfo> _vars;

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
  InputParameters _pars;

  /// finite element type for the flow in the simulation
  FEType _flow_fe_type;

  /// Spatial discretization
  FlowModel::ESpatialDiscretizationType _spatial_discretization;

  /**
   * Setup equations to be solved in this simulation
   */
  void setupEquations();

  /**
   * Setup reading initial conditions from a specified file, see 'initial_from_file' and
   * 'initial_from_file_timestep' parameters
   */
  void setupInitialConditionsFromFile();

  void setupInitialConditions();

  /**
   * Add proper element ghosting for parallel runs
   */
  void ghostElements();

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

public:
  Real _zero;

  friend class GlobalSimParamAction;
};

template <typename T>
const T &
Simulation::getParamTempl(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

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
