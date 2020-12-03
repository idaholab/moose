#pragma once

#include "GeneralReporter.h"

#include "OptimizeSolve.h"
#include "OptimizationParameterVectorPostprocessor.h"
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

  /**
   * Function to retrieve current parameters
   */
  dof_id_type getNumberOfParameters() const { return _ndof; }

protected:
  /// VPP to send data to
  OptimizationParameterVectorPostprocessor & _parameter_vpp;

  /// Const reference to vpp data
  std::vector<const VectorPostprocessorValue *> _parameters;

  /// Number of parameters
  dof_id_type _ndof = 0;

private:
  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  void updateParameters(const libMesh::PetscVector<Number> & x);

  friend class OptimizeSolve;
};
