#pragma once
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "ExternalProblem.h"
#include "MFEMMesh.h"
#include "MFEMCoefficient.h"
#include "MFEMVectorCoefficient.h"
#include "MFEMMaterial.h"
#include "MFEMVariable.h"
#include "MFEMScalarDirichletBC.h"
#include "MFEMConstantCoefficient.h"
#include "MFEMBoundaryCondition.h"
#include "MFEMKernel.h"
#include "MFEMExecutioner.h"
#include "MFEMDataCollection.h"
#include "MFEMFESpace.h"
#include "MFEMSolverBase.h"
#include "PropertyManager.h"
#include "Function.h"
#include "MooseEnum.h"
#include "SystemBase.h"
#include "Transient.h"
#include "Steady.h"
#include "platypus.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/point.h"

class MFEMProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  MFEMProblem(const InputParameters & params);
  virtual ~MFEMProblem(){};

  virtual void init() override;
  virtual void initialSetup() override;
  virtual void externalSolve() override{};
  virtual void outputStep(ExecFlagType type) override;
  virtual bool nlConverged(const unsigned int nl_sys_num) override { return true; };
  virtual void syncSolutions(Direction direction) override{};

  /**
   * Overwritten mesh() method from base MooseMesh to retrieve the correct mesh type, in this case
   * MFEMMesh.
   */
  virtual MFEMMesh & mesh() override;

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
   * Set the device to use to solve the FE problem.
   */
  void setDevice();

  /**
   * Set the mesh used by MFEM.
   */
  void setMesh(std::shared_ptr<mfem::ParMesh> pmesh);

  /**
   * Set the required ProblemBuilder to build a transient or steady state problem.
   */
  void setProblemBuilder();

  /**
   * Override of ExternalProblem::addVariable. Sets a
   * MFEM grid function to be used in the MFEM solve.
   */
  void addVariable(const std::string & var_type,
                   const std::string & var_name,
                   InputParameters & parameters) override;

  /**
   * Override of ExternalProblem::addAuxVariable. Sets a
   * MFEM grid function to be used in the MFEM solve.
   */
  void addAuxVariable(const std::string & var_type,
                      const std::string & var_name,
                      InputParameters & parameters) override;

  /**
   * Override of ExternalProblem::addKernel. Uses ExternalProblem::addKernel to create a
   * MFEMGeneralUserObject representing the kernel in MOOSE, and creates corresponding MFEM kernel
   * to be used in the MFEM solve.
   */
  void addKernel(const std::string & kernel_name,
                 const std::string & name,
                 InputParameters & parameters) override;

  /**
   * Method called in AddMFEMPreconditionerAction which will create the solver.
   */
  void addMFEMPreconditioner(const std::string & user_object_name,
                             const std::string & name,
                             InputParameters & parameters);

  /**
   * Method called in AddMFEMSolverAction which will create the solver.
   */
  void addMFEMSolver(const std::string & user_object_name,
                     const std::string & name,
                     InputParameters & parameters);

  /**
   * Add the nonlinear solver to the system. TODO: allow user to specify solver options,
   * similar to the linear solvers.
   */
  void addMFEMNonlinearSolver();

  /**
   * Method used to get an mfem FEC depending on the variable family specified in the input file.
   * This method is used in addAuxVariable to help create the MFEM grid function that corresponds to
   * a given MOOSE aux-variable.
   */
  InputParameters addMFEMFESpaceFromMOOSEVariable(InputParameters & moosevar_params);

  /**
   * Method to get the PropertyManager object for storing material
   * properties and converting them to MFEM coefficients. This is used
   * by Material and Kernel classes (among others).
   */
  platypus::PropertyManager & getProperties() { return _properties; }

  /**
   * Method to get the current platypus::Problem object storing the
   * current data specifying the FE problem.
   */
  platypus::Problem & getProblemData() { return *_problem_data; }

  platypus::Coefficients _coefficients;

protected:
  /**
   * Template method for adding kernels. We can only add kernels using equation system problem
   * builders.
   */
  template <class T>
  void addKernel(std::string var_name, std::shared_ptr<MFEMKernel<T>> kernel)
  {
    using namespace platypus;

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

  platypus::PropertyManager _properties;
  std::shared_ptr<platypus::Problem> _problem_data{nullptr};
  std::shared_ptr<platypus::ProblemBuilder> mfem_problem_builder{nullptr};
};
