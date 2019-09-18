//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LAROMData.h"

class SS316LAROMData;

template <>
InputParameters validParams<SS316LAROMData>();

class SS316LAROMData : public LAROMData
{
public:
  SS316LAROMData(const InputParameters & parameters);

  /// Returns index corresponding to the stress input
  virtual unsigned int getStressIndex() const override;

  /// Returns degree number for the Rom data set
  virtual unsigned int getDegree() const override;

  /// Returns the relative increment size limit for mobile dislocation density
  virtual Real getMaxRelativeMobileInc() const override;

  /// Returns the relative increment size limit for immobile dislocation density
  virtual Real getMaxRelativeImmobileInc() const override;

  /// Returns the relative increment size limit for the environmental factor
  virtual Real getMaxEnvironmentalFactorInc() const override;

  /* Returns vector of the functions to use for the conversion of input variables.
   * 0 = regular
   * 1 = log
   * 2 = exp
   */
  virtual std::vector<std::vector<unsigned int>> getTransform() const override;

  /// Returns factors for the functions for the conversion functions given in getTransform
  virtual std::vector<std::vector<Real>> getTransformCoefs() const override;

  /* Returns human-readable limits for the inputs. Inputs ordering is
   * 0: mobile
   * 1: immobile_old
   * 2: trial stress,
   * 3: effective strain old,
   * 4: temperature
   * 5: environmental factor (optional)
   */
  virtual std::vector<std::vector<Real>> getInputLimits() const override;

  /// Material specific coefficients multiplied by the Legendre polynomials for each of the input variables
  virtual std::vector<std::vector<Real>> getCoefs() const override;
};
