//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxiliarySystem.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "UserObject.h"

#include "libmesh/quadrature.h"

#include "FunctionSeries.h"
#include "MutableCoefficientsInterface.h"

/**
 * This class interacts with a MooseApp through functional expansions. It is templated to allow the
 * inheritance of two dual classes that operate in a volume (FXVolumeUserObject) or on a boundary
 * (FXBoundaryFluxUserObject and FXBoundaryValueUserObject)
 *
 * It uses an instance of FunctionSeries to generate the orthonormal function series required to
 * generate the functional expansion coefficients.
 */
template <class IntegralBaseVariableUserObject>
class FXIntegralBaseUserObject : public IntegralBaseVariableUserObject,
                                 public MutableCoefficientsInterface
{
public:
  FXIntegralBaseUserObject(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Return a reference to the underlying function series
   */
  const FunctionSeries & getFunctionSeries() const;

  // Override from <IntegralBaseVariableUserObject>
  virtual Real getValue() final;

  // Overrides from UserObject
  virtual void finalize() final;
  virtual void initialize() final;
  virtual Real spatialValue(const Point & location) const final;
  virtual void threadJoin(const UserObject & sibling) final;

protected:
  // Policy-based design requires us to specify which inherited members we are using
  using IntegralBaseVariableUserObject::_communicator;
  using IntegralBaseVariableUserObject::_console;
  using IntegralBaseVariableUserObject::_coord;
  using IntegralBaseVariableUserObject::_integral_value;
  using IntegralBaseVariableUserObject::_JxW;
  using IntegralBaseVariableUserObject::_q_point;
  using IntegralBaseVariableUserObject::_qp;
  using IntegralBaseVariableUserObject::_variable;
  using IntegralBaseVariableUserObject::computeIntegral;
  using IntegralBaseVariableUserObject::computeQpIntegral;
  using IntegralBaseVariableUserObject::getFunction;
  using IntegralBaseVariableUserObject::name;

  // Override from <IntegralBaseVariableUserObject>
  virtual Real computeIntegral() final;

  /**
   * Get the centroid of the evaluated unit
   */
  virtual Point getCentroid() const = 0;

  /**
   * Get the volume of the evaluated unit
   */
  virtual Real getVolume() const = 0;

  /// History of the expansion coefficients for each solve
  std::vector<std::vector<Real>> _coefficient_history;

  /// Current coefficient partial sums
  std::vector<Real> _coefficient_partials;

  /// Reference to the underlying function series
  FunctionSeries & _function_series;

  /// Keep the expansion coefficients after each solve
  const bool _keep_history;

  /// Flag to prints the state of the zeroth instance in finalize()
  const bool _print_state;

  /// Volume of the standardized functional space of integration
  const Real _standardized_function_volume;

  /// Moose volume of evaluation
  Real _volume;
};

template <class IntegralBaseVariableUserObject>
InputParameters
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::validParams()
{
  InputParameters params = IntegralBaseVariableUserObject::validParams();
  params += MutableCoefficientsInterface::validParams();

  params.addClassDescription(
      "This UserObject interacts with a MooseApp through functional expansions.");

  params.addRequiredParam<FunctionName>("function",
                                        "The name of the FunctionSeries \"Function\" object with "
                                        "which to generate this functional expansion.");

  params.addParam<bool>(
      "keep_history", false, "Keep the expansion coefficients from previous solves");

  params.addParam<bool>("print_state", false, "Print the state of the zeroth instance each solve");

  return params;
}

template <class IntegralBaseVariableUserObject>
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::FXIntegralBaseUserObject(
    const InputParameters & parameters)
  : IntegralBaseVariableUserObject(parameters),
    MutableCoefficientsInterface(this, parameters),
    _function_series(FunctionSeries::checkAndConvertFunction(
        getFunction("function"), UserObject::getParam<std::string>("_moose_base"), name())),
    _keep_history(UserObject::getParam<bool>("keep_history")),
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
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::computeIntegral()
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
    const std::vector<Real> & term_evaluations = _function_series.getGeneration();

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
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::finalize()
{
  // Sum the coefficient arrays over all processes
  _communicator.sum(_coefficient_partials);
  _communicator.sum(_volume);

  // Normalize the volume of the functional expansion to the FX standard space
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
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::getFunctionSeries() const
{
  return _function_series;
}

template <class IntegralBaseVariableUserObject>
Real
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::getValue()
{
  return _integral_value;
}

template <class IntegralBaseVariableUserObject>
void
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::initialize()
{
  IntegralBaseVariableUserObject::initialize();

  // Clear the partial sums
  for (auto & partial : _coefficient_partials)
    partial = 0;

  _volume = 0;
}

template <class IntegralBaseVariableUserObject>
void
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::threadJoin(const UserObject & s)
{
  const FXIntegralBaseUserObject<IntegralBaseVariableUserObject> & sibling =
      static_cast<const FXIntegralBaseUserObject<IntegralBaseVariableUserObject> &>(s);

  for (std::size_t c = 0; c < _coefficient_partials.size(); ++c)
    _coefficient_partials[c] += sibling._coefficient_partials[c];

  _volume += sibling._volume;
}

template <class IntegralBaseVariableUserObject>
Real
FXIntegralBaseUserObject<IntegralBaseVariableUserObject>::spatialValue(const Point & location) const
{
  _function_series.setLocation(location);

  return _function_series.expand(_coefficients);
}
