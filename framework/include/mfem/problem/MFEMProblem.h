#ifdef MFEM_ENABLED

#pragma once
#include <map>
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "ExternalProblem.h"
#include "MFEMProblemData.h"
#include "MFEMMesh.h"
#include "MFEMMaterial.h"
#include "MFEMVariable.h"
#include "MFEMBoundaryCondition.h"
#include "MFEMKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "MFEMExecutioner.h"
#include "MFEMDataCollection.h"
#include "MFEMFESpace.h"
#include "MFEMSolverBase.h"
#include "Function.h"
#include "MooseEnum.h"
#include "libmesh/string_to_enum.h"

class MFEMProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  MFEMProblem(const InputParameters & params);
  virtual ~MFEMProblem() {}

  virtual void initialSetup() override;
  virtual void externalSolve() override {}
  virtual bool nlConverged(const unsigned int) override { return true; }
  virtual void syncSolutions(Direction) override {}

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
                   InputParameters & parameters) override;

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
  void setMesh();

  /**
   * Initialise the required ProblemOperator used in the Executioner to solve the problem.
   */
  void initProblemOperator();

  /**
   * Override of ExternalProblem::addVariable. Sets a
   * MFEM grid function (and time derivative, for transient problems) to be used in the MFEM solve.
   */
  void addVariable(const std::string & var_type,
                   const std::string & var_name,
                   InputParameters & parameters) override;

  /**
   * Adds one MFEM GridFunction to be used in the MFEM solve.
   */
  void addGridFunction(const std::string & var_type,
                       const std::string & var_name,
                       InputParameters & parameters);

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
   * Override of ExternalProblem::addAuxKernel. Uses ExternalProblem::addAuxKernel to create a
   * MFEMGeneralUserObject representing the kernel in MOOSE, and creates corresponding MFEM kernel
   * to be used in the MFEM solve.
   */
  void addAuxKernel(const std::string & kernel_name,
                    const std::string & name,
                    InputParameters & parameters) override;

  /**
   * Override of ExternalProblem::addFunction. Uses ExternalProblem::addFunction to create a
   * MFEMGeneralUserObject representing the function in MOOSE, and creates a corresponding
   * MFEM Coefficient or VectorCoefficient object.
   */
  void addFunction(const std::string & type,
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
  MooseMFEM::PropertyManager & getProperties() { return _problem_data._properties; }

  /**
   * Method to get the current MFEMProblemData object storing the
   * current data specifying the FE problem.
   */
  MFEMProblemData & getProblemData() { return _problem_data; }

  /**
   * Method to build scalar mfem::Coefficient objects. These will be
   * tracked so that the simulation time can be updated on them later.
   */
  template <class T, class... Args>
  std::shared_ptr<T> makeScalarCoefficient(Args &&... args)
  {
    return this->_problem_data._scalar_manager.make<T>(args...);
  }

  /**
   * Method to build scalar mfem::VectorCoefficient objects. These will be
   * tracked so that the simulation time can be updated on them later.
   */
  template <class T, class... Args>
  std::shared_ptr<T> makeVectorCoefficient(Args &&... args)
  {
    return this->_problem_data._vector_manager.make<T>(args...);
  }

  /**
   * Method to build scalar mfem::MatrixCoefficient objects. These will be
   * tracked so that the simulation time can be updated on them later.
   */
  template <class T, class... Args>
  std::shared_ptr<T> makeMatrixCoefficient(Args &&... args)
  {
    return this->_problem_data._matrix_manager.make<T>(args...);
  }

  /**
   * Method to get the MFEM scalar coefficient object corresponding to the named function.
   */
  virtual std::shared_ptr<mfem::FunctionCoefficient>
  getScalarFunctionCoefficient(const std::string & name);

  /**
   * Method to get the MFEM vector coefficient object corresponding to the named function.
   */
  virtual std::shared_ptr<mfem::VectorFunctionCoefficient>
  getVectorFunctionCoefficient(const std::string & name);

  /**
   * Displace the mesh, if mesh displacement is enabled.
   */
  void displaceMesh();

  /**
   * Returns optional reference to the displacement GridFunction to apply to nodes.
   */
  std::optional<std::reference_wrapper<mfem::ParGridFunction const>>
  getMeshDisplacementGridFunction();

protected:
  /**
   * Template method for adding kernels. We can only add kernels using equation system problem
   * builders.
   */
  template <class T>
  void addKernel(std::string var_name, std::shared_ptr<MFEMKernel<T>> kernel)
  {
    using namespace MooseMFEM;
    if (getProblemData()._eqn_system)
    {
      getProblemData()._eqn_system->AddTrialVariableNameIfMissing(var_name);
      getProblemData()._eqn_system->AddKernel(var_name, std::move(kernel));
    }
    else
    {
      mooseError("Cannot add kernel with name '" + var_name +
                 "' because there is no equation system.");
    }
  }

  void addKernel(std::string trial_var_name,
                 std::string test_var_name,
                 std::shared_ptr<MFEMMixedBilinearFormKernel> kernel);

  MFEMProblemData _problem_data;
  std::map<std::string, std::shared_ptr<mfem::FunctionCoefficient>> _scalar_functions;
  std::map<std::string, std::shared_ptr<mfem::VectorFunctionCoefficient>> _vector_functions;
};

#endif
