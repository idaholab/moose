#pragma once

#include "GeneralReporter.h"

#include "OptimizeSolve.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class FormFunction : public GeneralReporter
{
public:
  static InputParameters validParams();
  FormFunction(const InputParameters & parameters);

  void initialize() override final {}
  void execute() override final {}
  void finalize() override final {}

  /**
   * Function to initialize petsc vectors from vpp data
   * FIXME: this should be const
   */
  void setInitialCondition(libMesh::PetscVector<Number> & param);

  /**
   * Function to compute objective.
   * This is the last function called in objective routine
   */
  virtual Real computeObjective() = 0;

  /**
   * Function to compute gradient.
   * This is the last call of the gradient routine.
   */
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient)
  {
    mooseError("Gradient function has not been defined for form function type ", _type);
  }

  /**
   * Function to compute gradient.
   * This is the last call of the hessian routine.
   */
  virtual void computeHessian(libMesh::PetscMatrix<Number> & hessian)
  {
    mooseError("Hessian function has not been defined for form function type ", _type);
  }

protected:
  /// Helper for getting or declaring data
  const std::vector<Real> & getDataValueHelper(const std::string & get_param,
                                               const std::string & declare_param);

  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameters
  const unsigned int _nparam;
  /// Number of values for each parameter
  const std::vector<dof_id_type> & _nvalues;
  /// Total number of degrees of freedom
  const dof_id_type _ndof;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;

private:
  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  void updateParameters(const libMesh::PetscVector<Number> & x);

  friend class OptimizeSolve;
};
