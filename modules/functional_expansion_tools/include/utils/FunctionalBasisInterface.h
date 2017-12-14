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

#ifndef FUNCTIONALBASISINTERFACE_H
#define FUNCTIONALBASISINTERFACE_H

// MOOSE includes
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseTypes.h"

// Shortened typename
class FunctionalBasisInterface;
typedef FunctionalBasisInterface FBI;

/// This class provides the basis for any custom functional basis
class FunctionalBasisInterface
{
public:
  /// default constructor
  FunctionalBasisInterface();

  /// non-default constructor
  FunctionalBasisInterface(const unsigned int number_of_terms);

  /// default destructor
  virtual ~FunctionalBasisInterface() = default;

  /// Returns the current evaluation at the given index
  Real operator[](std::size_t index) const;

  /// Returns true if the current evaluation is orthonormalized
  bool isOrthonormal() const;

  /// Returns true if the current evaluation is standardized
  bool isStandard() const;

  /// Returns an array reference containing the value of each orthonormalized term
  const std::vector<Real> & getAllOrthonormal();

  /// Returns an array reference containing the value of each standardized term
  const std::vector<Real> & getAllStandard();

  /// Returns the number of terms in the series
  std::size_t getNumberOfTerms() const;

  /// Gets the last term of the orthonormalized functional basis
  Real getOrthonormal();

  /// Gets the sum of all terms in the orthonormalized functional basis
  Real getOrthonormalSeriesSum();

  /// Gets the #_order-th term of the standardized functional basis
  Real getStandard();

  /// Evaluates the sum of all terms in the standardized functional basis up
  /// to #_order
  Real getStandardSeriesSum();

  /// An enumeration of the domains available to each functional series
  static MooseEnum _domain_options;

  // pure virtual methods to be defined by derived classes ----------------

  /// Whether the cached values correspond to the current point
  virtual bool isCacheInvalid() const = 0;

  /// Determines if the point provided is in within the physical bounds
  virtual bool isInPhysicalBounds(const Point & point) const = 0;

  /// Returns a vector of the lower and upper bounds of the standardized
  /// functional space
  virtual const std::vector<Real> & getStandardizedFunctionLimits() const = 0;

  /// Returns the volume within the standardized function local_limits
  virtual Real getStandardizedFunctionVolume() const = 0;

  /// Set the location that will be used by the series to compute values
  virtual void setLocation(const Point & point) = 0;

  /// Set the order of the series
  virtual void setOrder(const std::vector<std::size_t> & orders) = 0;

  /// Sets the bounds of the series
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) = 0;

  // pure virtual methods to be defined by derived classes ----------------

protected:
  /// Set all entries of the basis evaluation to zero.
  virtual void clearBasisEvaluation(const unsigned int & number_of_terms);

  /// Helper function to load a value from #_series
  Real load(std::size_t index) const;

  /// Helper function to store a value in #_series
  void save(std::size_t index, Real value);

  /// The number of terms in the series
  unsigned int _number_of_terms;

  /// indicates if the evaluated values correspond to the current location
  bool _is_cache_invalid;

  // pure virtual methods to be defined by derived classes ----------------

  /// Evaluate the orthonormal form of the functional basis
  virtual void evaluateOrthonormal() = 0;

  /// Evaluate the standardized form of the functional basis
  virtual void evaluateStandard() = 0;

  // pure virtual methods to be defined by derived classes ----------------

private:
  /// Stores the values of the basis evaluation
  std::vector<Real> _basis_evaluation;

  /// Indicates whether the current evaluation is standardized or orthonormalized
  bool _is_orthonormal;
};

#endif // FUNCTIONALBASISINTERFACE_H
