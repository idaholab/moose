#pragma once
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "ExternalProblem.h"
#include "ExclusiveMFEMMesh.h"
#include "CoupledMFEMMesh.h"
#include "MFEMMesh.h"
#include "MFEMCoefficient.h"
#include "MFEMVectorCoefficient.h"
#include "MFEMMaterial.h"
#include "MFEMVariable.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMConstantCoefficient.h"
#include "MFEMBoundaryCondition.h"
#include "MFEMBilinearFormKernel.h"
#include "MFEMLinearFormKernel.h"
#include "MFEMFormulation.h"
#include "MFEMAuxSolver.h"
#include "MFEMDataCollection.h"
#include "MFEMFESpace.h"
#include "Function.h"
#include "MooseEnum.h"
#include "SystemBase.h"
#include "Transient.h"
#include "Steady.h"
#include "hephaestus.hpp"
#include "libmesh/string_to_enum.h"
#include "libmesh/point.h"

class MFEMProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  MFEMProblem(const InputParameters & params);
  virtual ~MFEMProblem();

  virtual void init() override;
  virtual void initialSetup() override;
  virtual void externalSolve() override;
  virtual void outputStep(ExecFlagType type) override;

  virtual bool nlConverged(const unsigned int nl_sys_num) override { return true; };

  virtual void syncSolutions(Direction direction) override;

  /**
   * Overwritten mesh() method from base MooseMesh to retrieve the correct mesh type, in this case
   * ExclusiveMFEMMesh.
   */
  virtual ExclusiveMFEMMesh & mesh() override;

  /**
   * Returns all the variable names from the auxiliary system base. This is helpful in the
   * syncSolutions() method when transferring variable data.
   */
  virtual std::vector<VariableName> getAuxVariableNames();

  void addBoundaryCondition(const std::string & bc_name,
                            const std::string & name,
                            InputParameters & parameters) override;

  void addMaterial(const std::string & kernel_name,
                   const std::string & name,
                   InputParameters & parameters);

  /**
   * Add an MFEM coefficient to the problem.
   */
  void addCoefficient(const std::string & user_object_name,
                      const std::string & name,
                      InputParameters & parameters);

  /**
   * Add an MFEM vector coefficient to the problem.
   */
  void addVectorCoefficient(const std::string & user_object_name,
                            const std::string & name,
                            InputParameters & parameters);
  /**
   * Add an MFEM FESpace to the problem.
   */
  void addFESpace(const std::string & user_object_name,
                  const std::string & name,
                  InputParameters & parameters);
  /**
   * Override of ExternalProblem::addUserObject. Uses ExternalProblem::addUserObject to set the
   * Moose user objects, and contains additional code to create MFEM specific user objects.
   */
  void setFormulation(const std::string & user_object_name,
                      const std::string & name,
                      InputParameters & parameters);

  /**
   * Override of ExternalProblem::addAuxVariable. Uses ExternalProblem::addAuxVariable to set the
   * Moose aux var, and contains additional code to create a corresponding MFEM grid function to be
   * used in the MFEM solve.
   */
  void addAuxVariable(const std::string & var_type,
                      const std::string & var_name,
                      InputParameters & parameters) override;

  /**
   * Override of ExternalProblem::addKernel. Uses ExternalProblem::addKernel to create a
   * GeneralUserObject representing the kernel in MOOSE, and creates corresponding MFEM kernel
   * to be used in the MFEM solve.
   */
  void addKernel(const std::string & kernel_name,
                 const std::string & name,
                 InputParameters & parameters) override;
  /**
   * Override of ExternalProblem::addAuxKernel. Uses ExternalProblem::addAuxKernel to create a
   * GeneralUserObject representing the auxkernel in MOOSE, and creates corresponding MFEM auxsolver
   * to be used in the MFEM solve.
   */
  void addAuxKernel(const std::string & kernel_name,
                    const std::string & name,
                    InputParameters & parameters) override;

  /**
   * setMFEMVarData and setMOOSEVarData have very similar uses. They are both used to retrieve
   * data from one of the variable types (either Moose AuxVar or MFEM grid function), and
   * transfer it to the other. For example if you solve for temperature in MOOSE, you would use
   * setMFEMVarData to get this temperature data into an MFEM grid function.
   */
  void setMFEMVarData(const std::string & var_name);
  void setMOOSEVarData(const std::string & var_name);

  /**
   * Method used to get an mfem FEC depending on the variable family specified in the input file.
   * This method is used in addAuxVariable to help create the MFEM grid function that corresponds to
   * a given MOOSE aux-variable.
   */
  InputParameters addMFEMFESpaceFromMOOSEVariable(InputParameters & moosevar_params);

protected:
  /**
   * Called internally by setMFEMVarData.
   */
  void setMFEMNodalVarData(MooseVariableFieldBase & moose_variable);
  void setMFEMElementalVarData(MooseVariableFieldBase & moose_variable);

  /**
   * Called internally by setMOOSEVarData.
   */
  void setMOOSENodalVarData(MooseVariableFieldBase & moose_variable);
  void setMOOSEElementalVarData(MooseVariableFieldBase & moose_variable);

  /**
   * Template method for adding kernels. We can only add kernels using equation system problem
   * builders.
   */
  template <class T>
  void addKernel(std::string var_name, std::shared_ptr<hephaestus::Kernel<T>> kernel)
  {
    using namespace hephaestus;

    EquationSystemProblemBuilderInterface * eqn_system_problem_builder{nullptr};

    if ((eqn_system_problem_builder =
             dynamic_cast<EquationSystemProblemBuilderInterface *>(mfem_problem_builder.get())))
    {
      eqn_system_problem_builder->AddKernel(std::move(var_name), std::move(kernel));
    }
    else
    {
      mooseError("Cannot add kernel with name '" + var_name +
                 "' because there is no equation system.");
    }
  }

  std::string _input_mesh;
  std::string _formulation_name;
  int _order;

  hephaestus::Coefficients _coefficients;
  hephaestus::InputParameters _solver_options;
  hephaestus::Outputs _outputs;
  hephaestus::InputParameters _exec_params;

  std::shared_ptr<hephaestus::ProblemBuilder> mfem_problem_builder{nullptr};

  std::unique_ptr<hephaestus::Problem> mfem_problem{nullptr};
  std::unique_ptr<hephaestus::Executioner> executioner{nullptr};
};
