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

#ifndef FUNCTIONSERIES_H
#define FUNCTIONSERIES_H

// Module includes
#include "MutableCoefficientsFunctionInterface.h"
#include "CompositeSeriesBasisInterface.h"

// Forward declaration
class FunctionSeries;

// InputParameters declaration
template <>
InputParameters validParams<FunctionSeries>();

/**
 * This class uses implementations of CompositeSeriesBasisInterface to generate
 * a function based on comvolved function series. It is a grandchild of
 * MutableCoefficientsInterface, which enables easy MultiApp transfers of coefficients
 */
class FunctionSeries : public MutableCoefficientsFunctionInterface
{
public:
  FunctionSeries(const InputParameters & parameters);

  /// Static function to cast a Function to SeriesFunction
  static FunctionSeries & checkAndConvertFunction(Function & function, const std::string & name);

  virtual Real evaluateValue(Real t, const Point & p) override;

  /// Expand the function series at the current location and with the current coefficients
  Real expand();
  /// Expand the function using the provided coefficients at the current location
  Real expand(const std::vector<Real> & coefficients);

  std::size_t getNumberOfTerms() const;

  Real getStandardizedFunctionVolume() const;

  const std::vector<std::size_t> & getOrders() const;

  const std::vector<Real> & getOrthonormal();

  const std::vector<Real> & getStandard();

  bool isInPhysicalBounds(const Point & point) const;

  void setLocation(const Point & point);

  friend std::ostream & operator<<(std::ostream & stream, const FunctionSeries & me);

protected:
  const MooseEnum & _series_type_name;
  CompositeSeriesBasisInterface * _series_type;

  // vector holding the orders of each single series
  const std::vector<std::size_t> _orders;

  // the physical bounds of the function series
  const std::vector<Real> _physical_bounds;

  // enumerations of the possible series types for the different spatial
  // expansions. Not all of these will be provided for any one series.
  const MooseEnum & _x;
  const MooseEnum & _y;
  const MooseEnum & _z;
  const MooseEnum & _disc;

private:
  static std::vector<std::size_t> convertOrders(const std::vector<unsigned int> orders);
};

#endif
