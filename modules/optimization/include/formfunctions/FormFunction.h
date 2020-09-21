#pragma once

#include "MooseObject.h"

#include "OptimizationResults.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class FormFunction : public MooseObject
{
public:
  static InputParameters validParams();
  FormFunction(const InputParameters & parameters);

  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  void setParameters(const libMesh::PetscVector<Number> & x);

  /**
   * Function to compute objective.
   * This is the last function called in objective routine
   */
  virtual Real computeObjective() = 0;

  /**
   * Function to compute gradient.
   * This is the last call of the gradient routine.
   */
  virtual void computeGradient()
  {
    mooseError("Gradient function has not been defined for form function type ", _type);
  }

  /**
   * Function to compute gradient.
   * This is the last call of the hessian routine.
   */
  virtual void computeHessian()
  {
    mooseError("Hessian function has not been defined for form function type ", _type);
  }

  /**
   * Function to retrieve current parameters
   */
  dof_id_type getNumberOfParameters() const { return _ndof; }

  /**
   * Function to retrieve current parameters
   */
  libMesh::PetscVector<Number> & getParameters() { return _parameters; }
  const libMesh::PetscVector<Number> & getParameters() const { return _parameters; }

  /**
   * Function to retrieve last computed gradient
   */
  libMesh::PetscVector<Number> & getGradient() { return _gradient; }
  const libMesh::PetscVector<Number> & getGradient() const { return _gradient; }

  /**
   * Function to retrieve last computed gradient
   */
  libMesh::PetscMatrix<Number> & getHessian() { return _hessian; }
  const libMesh::PetscMatrix<Number> & getHessian() const { return _hessian; }

protected:
  /// Initial conditions
  const std::vector<Real> & _initial_condition;

  /// VPP to sent data to
  OptimizationResults & _results_vpp;

  /// Number of parameters
  const dof_id_type _ndof;

  /// Parameters
  libMesh::PetscVector<Number> _parameters;

  /// Gradient
  libMesh::PetscVector<Number> _gradient;

  /// Hessian
  libMesh::PetscMatrix<Number> _hessian;
};
