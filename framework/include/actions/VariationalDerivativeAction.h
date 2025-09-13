#pragma once

#include "Action.h"
#include "MooseAST.h"
#include "MooseExpressionBuilder.h"
#include "WeakFormGenerator.h"
#include "VariableSplitting.h"

class VariationalDerivativeAction : public Action
{
public:
  static InputParameters validParams();
  
  VariationalDerivativeAction(const InputParameters & params);
  
  virtual void act() override;
  
protected:
  void parseEnergyFunctional();
  
  void analyzeVariables();
  
  void setupVariableSplitting();
  
  void addPrimaryVariables();
  
  void addSplitVariables();
  
  void addKernels();
  
  void addAuxKernels();
  
  void addBoundaryConditions();
  
  void addPreconditioner();
  
  void addPostprocessors();
  
  void validateProblemSetup();
  
  void generateKernelForVariable(const std::string & var_name,
                                  const moose::automatic_weak_form::NodePtr & weak_form);
  
  void generateSplitVariableKernel(const moose::automatic_weak_form::SplitVariable & split_var);
  
  void setupCoupledProblem();
  
  void addJacobianBlocks();
  
  void optimizeSolverSettings();
  
private:
  enum class EnergyType
  {
    EXPRESSION,
    DOUBLE_WELL,
    ELASTIC_LINEAR,
    ELASTIC_NEOHOOKEAN,
    SURFACE_ISOTROPIC,
    SURFACE_ANISOTROPIC,
    CAHN_HILLIARD,
    CAHN_HILLIARD_FOURTH_ORDER,
    PHASE_FIELD_CRYSTAL,
    NAVIER_STOKES,
    CUSTOM
  };
  
  EnergyType _energy_type;
  
  std::string _energy_expression;
  
  std::vector<std::string> _variable_names;
  
  std::map<std::string, std::string> _variable_types;
  
  std::map<std::string, unsigned int> _variable_orders;
  
  std::vector<std::string> _coupled_variable_names;
  
  bool _enable_splitting;
  
  unsigned int _max_fe_order;
  
  bool _use_automatic_differentiation;
  
  bool _compute_jacobian_numerically;
  
  Real _fd_epsilon;
  
  bool _add_time_derivative;
  
  std::string _time_derivative_type;
  
  bool _enable_stabilization;
  
  std::string _stabilization_type;
  
  Real _stabilization_parameter;
  
  bool _symmetrize_jacobian;
  
  bool _use_preconditioner;
  
  std::string _preconditioner_type;
  
  bool _adaptive_splitting;
  
  Real _splitting_tolerance;
  
  bool _output_weak_form;
  
  std::string _weak_form_file;
  
  bool _validate_conservation;
  
  bool _check_stability;
  
  bool _estimate_condition_number;
  
  std::unique_ptr<moose::automatic_weak_form::MooseExpressionBuilder> _expr_builder;
  
  std::unique_ptr<moose::automatic_weak_form::WeakFormGenerator> _weak_form_gen;
  
  std::unique_ptr<moose::automatic_weak_form::VariableSplittingAnalyzer> _split_analyzer;
  
  std::unique_ptr<moose::automatic_weak_form::MixedFormulationGenerator> _mixed_gen;
  
  std::unique_ptr<moose::automatic_weak_form::VariationalProblemAnalyzer> _problem_analyzer;
  
  moose::automatic_weak_form::NodePtr _energy_functional;
  
  std::map<std::string, moose::automatic_weak_form::NodePtr> _weak_forms;
  
  std::map<std::string, moose::automatic_weak_form::NodePtr> _multiple_energies;
  
  std::map<std::string, moose::automatic_weak_form::SplitVariable> _split_variables;
  
  std::map<std::string, std::map<std::string, moose::automatic_weak_form::NodePtr>> _jacobian_blocks;
  
  struct VariableInfo
  {
    std::string name;
    std::string family;
    std::string order;
    bool is_vector;
    bool is_tensor;
    unsigned int components;
    bool is_auxiliary;
    bool is_split;
    std::string parent_variable;
  };
  
  std::map<std::string, VariableInfo> _variable_info;
  
  struct KernelInfo
  {
    std::string name;
    std::string type;
    std::string variable;
    std::vector<std::string> coupled_vars;
    moose::automatic_weak_form::NodePtr expression;
    bool is_auxiliary;
  };
  
  std::vector<KernelInfo> _kernels;
  
  void buildEnergyFromType();
  
  void buildDoubleWellEnergy();
  void buildElasticEnergy();
  void buildNeoHookeanEnergy();
  void buildSurfaceEnergy();
  void buildCahnHilliardEnergy();
  void buildFourthOrderCahnHilliardEnergy();
  void buildPhaseFieldCrystalEnergy();
  void buildNavierStokesEnergy();
  
  void setupMaterialProperties();
  
  void addMaterialProperty(const std::string & name, const std::string & type);
  
  std::string generateKernelName(const std::string & var_name, const std::string & suffix = "");
  
  std::string determineVariableFamily(const moose::automatic_weak_form::Shape & shape,
                                       unsigned int order);
  
  void writeWeakFormToFile();
  
  void performConservationCheck();
  
  void performStabilityAnalysis();
  
  void estimateSystemConditionNumber();
  
  void reportAnalysisResults();
  
  void setupAdaptiveSplitting();
  
  void addErrorIndicators();
  
  void addAdaptivitySystem();
};

class VariationalProblemAction : public Action
{
public:
  static InputParameters validParams();
  
  VariationalProblemAction(const InputParameters & params);
  
  virtual void act() override;
  
private:
  void setupProblem();
  void addSubActions();
  
  std::string _problem_type;
  std::vector<std::string> _physics;
  bool _automatic_setup;
};

class CoupledVariationalAction : public Action
{
public:
  static InputParameters validParams();
  
  CoupledVariationalAction(const InputParameters & params);
  
  virtual void act() override;
  
private:
  void setupCoupledSystem();
  
  void addCoupledKernels();
  
  void setupBlockSolver();
  
  std::vector<std::string> _energy_components;
  std::map<std::string, std::vector<std::string>> _component_variables;
  std::map<std::string, std::string> _coupling_terms;
  
  bool _use_block_solver;
  std::string _block_solver_type;
};