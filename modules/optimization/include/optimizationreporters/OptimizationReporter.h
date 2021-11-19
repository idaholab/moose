#pragma once

#include "GeneralReporter.h"

#include "OptimizeSolve.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "DataIO.h"

class OptimizationReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  OptimizationReporter(const InputParameters & parameters);

  void initialize() override final {}
  void execute() override final {}
  void finalize() override final {}

  /**
   * Function to initialize petsc vectors from vpp data
   * FIXME: this should be const
   */
  void setInitialCondition(libMesh::PetscVector<Number> & param);

  /**
   * Functions to get and check bounds
   */
  bool hasBounds() const { return _upper_bounds.size() > 0 && _lower_bounds.size() > 0; }
  const std::vector<Real> & getUpperBounds() const { return _upper_bounds; };
  const std::vector<Real> & getLowerBounds() const { return _lower_bounds; };
  /**
   * Function to compute default bounds when user did not provide bounds
   */
  virtual std::vector<Real> computeDefaultBounds(Real val);

  /**
   * Function to compute objective and handle a failed solve.
   * This is the last function called in objective routine
   */
  virtual Real computeAndCheckObjective(bool solver_converged);

  /**
   * Function to compute gradient.
   * This is the last call of the gradient routine.
   */
  virtual void computeGradient(libMesh::PetscVector<Number> & /*gradient*/)
  {
    mooseError("Gradient function has not been defined for form function type ", _type);
  }

  /**
   * Function to compute gradient.
   * This is the last call of the hessian routine.
   */
  virtual void computeHessian(libMesh::PetscMatrix<Number> & /*hessian*/)
  {
    mooseError("Hessian function has not been defined for form function type ", _type);
  }

  /**
   * Function to get the total number of parameters
   */
  unsigned int getNumParams() { return _ndof; };

protected:
  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameter vectors
  const unsigned int _nparam;
  /// Number of values for each parameter
  const std::vector<dof_id_type> & _nvalues;
  /// Total number of parameters
  const dof_id_type _ndof;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;

  /// Bounds of the parameters
  const std::vector<Real> & _lower_bounds;
  const std::vector<Real> & _upper_bounds;

  /// Measurement points and values
  const std::vector<Real> & _measurement_values;
  const std::vector<Point> & _measurement_points;
  // fixme this should be a struct.  The order is measurement point, measurement value, simulation
  // value, misfit
  std::vector<std::tuple<Point, Real, Real, Real>> & _measurement_data;

  /// vector of misfit data
  const std::vector<Real> & _misfit;

  /**
   * Function to compute objective.
   * This is the last function called in objective routine
   */
  Real computeObjective();

  /**
   * Function to set parameters.
   * This is the first function called in objective/gradient/hessian routine
   */
  virtual void updateParameters(const libMesh::PetscVector<Number> & x);

private:
  friend class OptimizeSolve;
};

// fixme
// nothing below is implemented and it probalby should be
namespace libMesh
{
void to_json(nlohmann::json & json, const std::vector<std::tuple<Point, Real, Real, Real>> & value);
}

/**
 * Store and load methods for measurement_data tuple,
 * which do nothing because I don't know if we need them
 */
///@{
template <>
void dataStore(std::ostream & stream, std::tuple<Point, Real, Real, Real> & v, void * context);
template <>
void dataLoad(std::istream & stream, std::tuple<Point, Real, Real, Real> & v, void * context);
///@}
