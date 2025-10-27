//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "ExternalProblem.h"
#include "MFEMProblemData.h"
#include "MFEMMesh.h"

class MFEMProblem : public ExternalProblem
{
public:
  static InputParameters validParams();

  MFEMProblem(const InputParameters & params);
  virtual ~MFEMProblem() {}

  virtual void initialSetup() override;
  virtual void externalSolve() override {}
  virtual void syncSolutions(Direction) override {}

  /**
   * Overwritten mesh() method from base MooseMesh to retrieve the correct mesh type, in this case
   * MFEMMesh.
   */
  virtual MFEMMesh & mesh() override;
  virtual const MFEMMesh & mesh() const override;
  using ExternalProblem::mesh;

  /**
   * Returns all the variable names from the auxiliary system base. This is helpful in the
   * syncSolutions() method when transferring variable data.
   */
  virtual std::vector<VariableName> getAuxVariableNames();

  void addBoundaryCondition(const std::string & bc_name,
                            const std::string & name,
                            InputParameters & parameters) override;

  void addMaterial(const std::string & material_name,
                   const std::string & name,
                   InputParameters & parameters) override;

  void addFunctorMaterial(const std::string & material_name,
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
   * Add an MFEM SubMesh to the problem.
   */
  void addSubMesh(const std::string & user_object_name,
                  const std::string & name,
                  InputParameters & parameters);

  /**
   * Add transfers between MultiApps and/or MFEM SubMeshes.
   */
  void addTransfer(const std::string & transfer_name,
                   const std::string & name,
                   InputParameters & parameters) override;
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

  using ExternalProblem::addAuxVariable;
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

  void addInitialCondition(const std::string & ic_name,
                           const std::string & name,
                           InputParameters & parameters) override;

  /**
   * Override of ExternalProblem::addPostprocessor. In addition to
   * creating the postprocessor object, it will create a coefficient
   * that will hold its value.
   */
  void addPostprocessor(const std::string & type,
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
  Moose::MFEM::CoefficientManager & getCoefficients() { return _problem_data.coefficients; }

  /**
   * Method to get the current MFEMProblemData object storing the
   * current data specifying the FE problem.
   */
  MFEMProblemData & getProblemData() { return _problem_data; }

  /**
   * Return the MPI communicator associated with this FE problem's mesh.
   */
  MPI_Comm getComm() { return mesh().getMFEMParMesh().GetComm(); }

  /**
   * Displace the mesh, if mesh displacement is enabled.
   */
  void displaceMesh();

  /**
   * Returns optional reference to the displacement GridFunction to apply to nodes.
   */
  std::optional<std::reference_wrapper<mfem::ParGridFunction const>>
  getMeshDisplacementGridFunction();

  Moose::FEBackend feBackend() const override { return Moose::FEBackend::MFEM; }

  std::string solverTypeString(unsigned int solver_sys_num) override;

  /**
   * @returns a shared pointer to an MFEM parallel grid function
   */
  std::shared_ptr<mfem::ParGridFunction> getGridFunction(const std::string & name);

  /**
   * set newton solver parameters
   */
  void setNewtonParamaters(unsigned int nl_max_its, mfem::real_t nl_abs_tol, mfem::real_t nl_rel_tol, unsigned int print_level);

protected:
  MFEMProblemData _problem_data;
  unsigned int _nl_max_its;
  mfem::real_t _nl_abs_tol;
  mfem::real_t _nl_rel_tol;
  unsigned int _print_level;
};

#endif
