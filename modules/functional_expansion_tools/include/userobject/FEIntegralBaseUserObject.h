/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef FEINTEGRALBASEUSEROBJECT_H
#define FEINTEGRALBASEUSEROBJECT_H

// MOOSE includes
#include "AuxiliarySystem.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "UserObject.h"

// libMesh includes
#include "libmesh/quadrature.h"

// Module includes
#include "FunctionSeries.h"
#include "MutableCoefficientsInterface.h"

// Class declaration for parameters - we cannot use templated types in validParams<>()
class FEIntegralBaseUserObjectParameters
{
  // Empty class, used only for parameters
};

template <>
InputParameters validParams<FEIntegralBaseUserObjectParameters>();

/**
 * This class interacts with a MooseApp through functional expansions. It is templated to allow the
 * inheritance of two dual classes that operate in a volume (FEVolumeUserObject) or on a boundary
 * (FEBoundaryFluxUserObject and FEBoundaryValueUserObject)
 *
 * It uses an instance of FunctionSeries to generate the orthonormal function series required to
 * generate the functional expansion coefficients.
 */
template <class IntegralBaseVariableUserObject>
class FEIntegralBaseUserObject : public IntegralBaseVariableUserObject,
                                 public MutableCoefficientsInterface
{
public:
  FEIntegralBaseUserObject(const InputParameters & parameters);

  /// Return a reference to the underlying function series
  const FunctionSeries & getFunctionSeries() const;

  // Override from <IntegralBaseVariableUserObject>
  virtual Real getValue() final;

  // Overrides from UserObject
  virtual void finalize() final;
  virtual void initialize() final;
  virtual Real spatialValue(const Point & location) const final;
  virtual void threadJoin(const UserObject & sibling) final;

  /// Keep the expansion coefficients after each solve
  const bool _keep_history;

protected:
  // Policy-based design requires us to specify which inherited members we are using
  using IntegralBaseVariableUserObject::_JxW;
  using IntegralBaseVariableUserObject::_communicator;
  using IntegralBaseVariableUserObject::_console;
  using IntegralBaseVariableUserObject::_coord;
  using IntegralBaseVariableUserObject::_integral_value;
  using IntegralBaseVariableUserObject::_q_point;
  using IntegralBaseVariableUserObject::_qp;
  using IntegralBaseVariableUserObject::_variable;
  using IntegralBaseVariableUserObject::computeIntegral;
  using IntegralBaseVariableUserObject::computeQpIntegral;
  using IntegralBaseVariableUserObject::getFunction;
  using IntegralBaseVariableUserObject::name;

  /// Override from <IntegralBaseVariableUserObject>
  virtual Real computeIntegral() final;

  /// Pure virtual method to get the centroid of the evaluated unit
  virtual Point getCentroid() const = 0;

  /// Pure virtual method to get the volume
  virtual Real getVolume() const = 0;

  /// History of the expansion coefficients for each solve
  std::vector<std::vector<Real>> _coefficient_history;

  /// Current coefficient partial sums
  std::vector<Real> _coefficient_partials;

  /// Reference to the underlying function series
  FunctionSeries & _function_series;

  /// Flag to prints the state of the zeroth instance in finalize()
  const bool _print_state;

  /// Volume of the standardized functional space of integration
  const Real _standardized_function_volume;

  /// Moose volume of evaluation
  Real _volume;
};

template <class IntegralBaseVariableUserObject>
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::FEIntegralBaseUserObject(
    const InputParameters & parameters)
  : IntegralBaseVariableUserObject(parameters),
    MutableCoefficientsInterface(parameters),
    _keep_history(UserObject::getParam<bool>("keep_history")),
    _function_series(FunctionSeries::checkAndConvertFunction(getFunction("function"), name())),
    _print_state(UserObject::getParam<bool>("print_state")),
    _standardized_function_volume(_function_series.getStandardizedFunctionVolume())
{
  // Size the coefficient arrays
  _coefficient_partials.resize(_function_series.getNumberOfTerms(), 0.0);
  _coefficients.resize(_function_series.getNumberOfTerms(), 0.0);
  _characteristics = _function_series.getOrders();

  if (!_keep_history)
    _coefficient_history.resize(0);
}

template <class IntegralBaseVariableUserObject>
Real
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::computeIntegral()
{
  Real sum = 0.0;
  const Point centroid = getCentroid();

  // Check to see if this element/side is within the valid boundaries
  if (!_function_series.isInPhysicalBounds(centroid))
    return 0.0;

  // Loop over the quadrature points
  for (_qp = 0; _qp < _q_point.size(); ++_qp)
  {
    // Get the functional terms for a vectorized approach
    _function_series.setLocation(_q_point[_qp]);
    const std::vector<Real> & term_evaluations = _function_series.getOrthonormal();

    // Evaluate the functional expansion coefficients at each quadrature point
    const Real local_contribution = computeQpIntegral();
    const Real common_evaluation = local_contribution * _JxW[_qp] * _coord[_qp];
    for (std::size_t c = 0; c < _coefficient_partials.size(); ++c)
      _coefficient_partials[c] += term_evaluations[c] * common_evaluation;

    sum += local_contribution;
  }

  _volume += getVolume();

  return sum;
}

template <class IntegralBaseVariableUserObject>
void
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::finalize()
{
  // Sum the coefficient arrays over all processes
  _communicator.sum(_coefficient_partials);
  _communicator.sum(_volume);

  // Normalize the volume of the functional expansion to the FE standard space
  const Real volume_normalization = _standardized_function_volume / _volume;
  for (auto & partial : _coefficient_partials)
    partial *= volume_normalization;

  // We now have the completely evaluated coefficients
  _coefficients = _coefficient_partials;

  // The average value is the same as the zeroth coefficient
  _integral_value = _coefficient_partials[0];

  if (_keep_history)
    _coefficient_history.push_back(_coefficients);

  if (_print_state)
  {
    _function_series.setCoefficients(_coefficients);
    _console << COLOR_YELLOW << _function_series << COLOR_DEFAULT << std::endl;
  }
}

template <class IntegralBaseVariableUserObject>
const FunctionSeries &
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::getFunctionSeries() const
{
  return _function_series;
}

template <class IntegralBaseVariableUserObject>
Real
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::getValue()
{
  return _integral_value;
}

template <class IntegralBaseVariableUserObject>
void
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::initialize()
{
  IntegralBaseVariableUserObject::initialize();

  // Clear the partial sums
  for (auto & partial : _coefficient_partials)
    partial = 0;

  _volume = 0;
}

template <class IntegralBaseVariableUserObject>
void
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::threadJoin(const UserObject & s)
{
  const FEIntegralBaseUserObject<IntegralBaseVariableUserObject> & sibling =
      static_cast<const FEIntegralBaseUserObject<IntegralBaseVariableUserObject> &>(s);

  for (std::size_t c = 0; c < _coefficient_partials.size(); ++c)
    _coefficient_partials[c] += sibling._coefficient_partials[c];

  _volume += sibling._volume;
}

template <class IntegralBaseVariableUserObject>
Real
FEIntegralBaseUserObject<IntegralBaseVariableUserObject>::spatialValue(const Point & location) const
{
  _function_series.setLocation(location);

  return _function_series.expand(_coefficients);
}

#endif // FEINTEGRALBASEUSEROBJECT_H
