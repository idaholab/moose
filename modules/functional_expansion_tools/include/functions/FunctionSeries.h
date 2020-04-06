//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MutableCoefficientsFunctionInterface.h"
#include "CompositeSeriesBasisInterface.h"

/**
 * This class uses implementations of CompositeSeriesBasisInterface to generate a function based on
 * convolved function series. Its inheritance tree includes MutableCoefficientsInterface, which
 * enables easy MultiApp transfers of coefficients.
 */
class FunctionSeries : public MutableCoefficientsFunctionInterface
{
public:
  static InputParameters validParams();

  FunctionSeries(const InputParameters & parameters);

  /**
   * Static function to cast a Function to SeriesFunction
   */
  static FunctionSeries & checkAndConvertFunction(const Function & function,
                                                  const std::string & typeName,
                                                  const std::string & objectName);

  // Override from MemoizedFunctionInterface
  virtual Real evaluateValue(Real t, const Point & p) override;

  /**
   * Expand the function series at the current location and with the current coefficients
   */
  Real expand();

  /**
   * Expand the function using the provided coefficients at the current location
   */
  Real expand(const std::vector<Real> & coefficients);

  /**
   * Returns the number of terms (coefficients) in the underlying function series
   */
  std::size_t getNumberOfTerms() const;

  /**
   * Returns the volume of evaluation in the functional series standardized space
   */
  Real getStandardizedFunctionVolume() const;

  /**
   * Returns a vector of the functional orders in the underlying functional series
   */
  const std::vector<std::size_t> & getOrders() const;

  /**
   * Returns a vector of the generation-evaluated functional series at the current location
   */
  const std::vector<Real> & getGeneration();

  /**
   * Returns a vector of the expansion-evaluated functional series at the current location
   */
  const std::vector<Real> & getExpansion();

  /**
   * Returns true if the provided point is within the set physical boundaries
   */
  bool isInPhysicalBounds(const Point & point) const;

  /**
   * Set the current evaluation location
   */
  void setLocation(const Point & point);

  /**
   * Returns a tabularized text stream of the currently stored coefficients
   */
  friend std::ostream & operator<<(std::ostream & stream, const FunctionSeries & me);

protected:
  /// The vector holding the orders of each single series
  const std::vector<std::size_t> _orders;

  /// The physical bounds of the function series
  const std::vector<Real> _physical_bounds;

  /// Stores a pointer to the functional series object
  std::unique_ptr<CompositeSeriesBasisInterface> _series_type;

  /// Stores the name of the current functional series type
  const MooseEnum & _series_type_name;

  /*
   * Enumerations of the possible series types for the different spatial expansions. Not all of
   * these will be provided for any one series.
   */
  /// Stores the name of the single function series to use in the x direction
  const MooseEnum & _x;
  /// Stores the name of the single function series to use in the y direction
  const MooseEnum & _y;
  /// Stores the name of the single function series to use in the z direction
  const MooseEnum & _z;
  /// Stores the name of the single function series to use for a unit disc
  const MooseEnum & _disc;
  /// The normalization type for expansion
  const MooseEnum & _expansion_type;
  /// The normalization type for generation
  const MooseEnum & _generation_type;

private:
  /**
   * Static function to convert an array of `unsigned int` to `std::size_t`. The MOOSE parser has
   * issues reading a list of integers in as `std::size_t` (unsigned long), so this workaround is
   * required in order to set `_orders` in the constructor initializer list.
   */
  static std::vector<std::size_t> convertOrders(const std::vector<unsigned int> & orders);
};
