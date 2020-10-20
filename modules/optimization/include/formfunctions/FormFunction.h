#pragma once

#include "MooseObject.h"

#include "OptimizationVectorPostprocessor.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class FormFunction : public MooseObject, public VectorPostprocessorInterface
{
public:
  static InputParameters validParams();
  FormFunction(const InputParameters & parameters);

  /**
   * Function to initialize petsc vectors from vpp data
   */
  void initializePetscVectors();

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

  /**
   * Get communicator used for matrices
   */
  const libMesh::Parallel::Communicator & getComm() const { return _my_comm; }

protected:
  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// VPP to sent data to
  OptimizationVectorPostprocessor & _results_vpp;

  /// VPP containing the measurement data
  const VectorPostprocessorValue & _measurement_vpp_values;

  /// Number of parameters
  dof_id_type _ndof;

  /// Parameters
  libMesh::PetscVector<Number> _parameters;

  /// Gradient
  libMesh::PetscVector<Number> _gradient;

  /// Hessian
  libMesh::PetscMatrix<Number> _hessian;
};
