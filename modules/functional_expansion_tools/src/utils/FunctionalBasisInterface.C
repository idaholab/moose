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

#include "FunctionalBasisInterface.h"

MooseEnum FunctionalBasisInterface::_domain_options("x=0 y=1 z=2");

/*
 * The default constructor is used to initialize a series before knowing the number of terms in the
 * series. This is called from the CSBI, and in the body of the CSBI constructor, setNumberOfTerms()
 * is used to after-the-fact perform the same initializations that would be done with the
 * non-default constructor.
 */
FunctionalBasisInterface::FunctionalBasisInterface()
  : _is_cache_invalid(true), _is_orthonormal(false)
{
}

/*
 * The non-default constructor should be used to initialize a series if the number of terms is
 * known, such as with a single series.
 */
FunctionalBasisInterface::FunctionalBasisInterface(const unsigned int number_of_terms)
  : _number_of_terms(number_of_terms),
    _is_cache_invalid(true),
    _basis_evaluation(_number_of_terms, 0.0),
    _is_orthonormal(false)
{
  _basis_evaluation.shrink_to_fit();
}

Real FunctionalBasisInterface::operator[](std::size_t index) const
{
  return (index > _basis_evaluation.size() ? 0.0 : _basis_evaluation[index]);
}

bool
FunctionalBasisInterface::isOrthonormal() const
{
  return _is_orthonormal;
}

bool
FunctionalBasisInterface::isStandard() const
{
  return !_is_orthonormal;
}

const std::vector<Real> &
FunctionalBasisInterface::getAllOrthonormal()
{
  if (isStandard() || isCacheInvalid())
  {
    clearBasisEvaluation(_number_of_terms);

    evaluateOrthonormal();

    _is_orthonormal = true;
    _is_cache_invalid = false;
  }

  return _basis_evaluation;
}

const std::vector<Real> &
FunctionalBasisInterface::getAllStandard()
{
  if (isOrthonormal() || isCacheInvalid())
  {
    clearBasisEvaluation(_number_of_terms);

    evaluateStandard();

    _is_orthonormal = false;
    _is_cache_invalid = false;
  }

  return _basis_evaluation;
}

std::size_t
FunctionalBasisInterface::getNumberOfTerms() const
{
  return _number_of_terms;
}

Real
FunctionalBasisInterface::getOrthonormal()
{
  // Use getAllOrthonormal() which will lazily evaluate the series as needed
  return getAllOrthonormal().back();
}

Real
FunctionalBasisInterface::getOrthonormalSeriesSum()
{
  Real sum = 0.0;

  // Use getAllOrthonormal() which will lazily evaluate the series as needed
  for (auto term : getAllOrthonormal())
    sum += term;

  return sum;
}

Real
FunctionalBasisInterface::getStandard()
{
  // Use getAllStandard() which will lazily evaluate the series as needed
  return getAllStandard().back();
}

Real
FunctionalBasisInterface::getStandardSeriesSum()
{
  Real sum = 0.0;

  // Use getAllStandard() which will lazily evaluate the series as needed
  for (auto term : getAllStandard())
    sum += term;

  return sum;
}

Real
FunctionalBasisInterface::load(std::size_t index) const
{
  return _basis_evaluation[index];
}

void
FunctionalBasisInterface::save(std::size_t index, Real value)
{
  _basis_evaluation[index] = value;
}

void
FunctionalBasisInterface::clearBasisEvaluation(const unsigned int & number_of_terms, SBIKey)
{
  clearBasisEvaluation(number_of_terms);
}

void
FunctionalBasisInterface::clearBasisEvaluation(const unsigned int & number_of_terms)
{
  _basis_evaluation.assign(number_of_terms, 0.0);
  _basis_evaluation.shrink_to_fit();
}
