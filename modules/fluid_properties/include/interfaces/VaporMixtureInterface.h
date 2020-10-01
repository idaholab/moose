//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "InitialCondition.h"
#include "VaporMixtureFluidProperties.h"

template <typename T = InitialCondition>
class VaporMixtureInterface;

/**
 * Interface for calculations involving vapor mixtures
 */
template <class T>
class VaporMixtureInterface : public T
{
public:
  VaporMixtureInterface(const InputParameters & parameters);

protected:
  /**
   * Gets the vector of mass fractions of each secondary vapor, at a quadrature point
   */
  std::vector<Real> getMassFractionVector() const;

  /// Vapor mixture fluid properties
  const VaporMixtureFluidProperties & _fp_vapor_mixture;
  /// Number of secondary vapors
  const unsigned int _n_secondary_vapors;
  /// Mass fractions of secondary vapors
  const std::vector<const VariableValue *> _x_secondary_vapors;

public:
  static InputParameters validParams();
};

template <class T>
InputParameters
VaporMixtureInterface<T>::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredCoupledVar("x_secondary_vapors", "Mass fractions of secondary vapors");
  params.addRequiredParam<UserObjectName>("fp_vapor_mixture",
                                          "Vapor mixture fluid properties user object name");

  return params;
}

template <class T>
VaporMixtureInterface<T>::VaporMixtureInterface(const InputParameters & parameters)
  : T(parameters),
    _fp_vapor_mixture(T::template getUserObject<VaporMixtureFluidProperties>("fp_vapor_mixture")),
    _n_secondary_vapors(T::coupledComponents("x_secondary_vapors")),
    _x_secondary_vapors(T::coupledValues("x_secondary_vapors"))
{
}

template <class T>
std::vector<Real>
VaporMixtureInterface<T>::getMassFractionVector() const
{
  std::vector<Real> x_secondary_vapors(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    x_secondary_vapors[i] = (*_x_secondary_vapors[i])[T::_qp];
  return x_secondary_vapors;
}
