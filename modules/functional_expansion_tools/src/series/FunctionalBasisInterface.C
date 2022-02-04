//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionalBasisInterface.h"

MooseEnum FunctionalBasisInterface::_domain_options("x=0 y=1 z=2");

/*
 * The default constructor is used to initialize a series before knowing the number of terms in the
 * series. This is called from the CSBI, and in the body of the CSBI constructor, setNumberOfTerms()
 * is used to after-the-fact perform the same initializations that would be done with the
 * non-default constructor.
 */
FunctionalBasisInterface::FunctionalBasisInterface()
  : _is_cache_invalid(true), _is_generation(false)
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
    _is_generation(false)
{
  _basis_evaluation.shrink_to_fit();
}

Real
FunctionalBasisInterface::operator[](std::size_t index) const
{
  return (index > _basis_evaluation.size() ? 0.0 : _basis_evaluation[index]);
}

bool
FunctionalBasisInterface::isGeneration() const
{
  return _is_generation;
}

bool
FunctionalBasisInterface::isExpansion() const
{
  return !_is_generation;
}

const std::vector<Real> &
FunctionalBasisInterface::getAllGeneration()
{
  if (isExpansion() || isCacheInvalid())
  {
    clearBasisEvaluation(_number_of_terms);

    evaluateGeneration();

    _is_generation = true;
    _is_cache_invalid = false;
  }

  return _basis_evaluation;
}

const std::vector<Real> &
FunctionalBasisInterface::getAllExpansion()
{
  if (isGeneration() || isCacheInvalid())
  {
    clearBasisEvaluation(_number_of_terms);

    evaluateExpansion();

    _is_generation = false;
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
FunctionalBasisInterface::getGeneration()
{
  // Use getAllGeneration() which will lazily evaluate the series as needed
  return getAllGeneration().back();
}

Real
FunctionalBasisInterface::getGenerationSeriesSum()
{
  Real sum = 0.0;

  // Use getAllGeneration() which will lazily evaluate the series as needed
  for (auto term : getAllGeneration())
    sum += term;

  return sum;
}

Real
FunctionalBasisInterface::getExpansion()
{
  // Use getAllExpansion() which will lazily evaluate the series as needed
  return getAllExpansion().back();
}

Real
FunctionalBasisInterface::getExpansionSeriesSum()
{
  Real sum = 0.0;

  // Use getAllExpansion() which will lazily evaluate the series as needed
  for (auto term : getAllExpansion())
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
FunctionalBasisInterface::clearBasisEvaluation(const unsigned int & number_of_terms)
{
  _basis_evaluation.assign(number_of_terms, 0.0);
  _basis_evaluation.shrink_to_fit();
}
