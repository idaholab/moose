//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class LAROMData;

template <>
InputParameters validParams<LAROMData>();

class LAROMData : public GeneralUserObject
{
public:
  LAROMData(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override {}
  virtual void finalize() override {}

  /// Calculates and returns the number of inputs for the ROM data set
  unsigned int getNumberInputs() const;

  /// Calculates and returns the number of outputs for the ROM data set
  unsigned int getNumberOutputs() const;

  /// Calculates and returns the number of ROM coefficients for the ROM data set
  unsigned int getNumberRomCoefficients() const;

  /// Checks to number of inputs to see if the environmental factor is included
  bool checkForEnvironmentFactor() const;

  /// Calculates and returns the transformed limits for the ROM calculations
  std::vector<std::vector<std::vector<Real>>> getTransformedLimits() const;

  /// Calculates and returns vector utilized in assign values
  std::vector<std::vector<unsigned int>> getMakeFrameHelper() const;

  /// Returns index corresponding to the stress input
  virtual unsigned int getStressIndex() const = 0;

  /// Returns degree number for the Rom data set
  virtual unsigned int getDegree() const = 0;

  /// Returns the relative increment size limit for mobile dislocation density
  virtual Real getMaxRelativeMobileInc() const = 0;

  /// Returns the relative increment size limit for immobile dislocation density
  virtual Real getMaxRelativeImmobileInc() const = 0;

  /// Returns the relative increment size limit for the environmental factor
  virtual Real getMaxEnvironmentalFactorInc() const = 0;

  /* Returns vector of the functions to use for the conversion of input variables.
   * 0 = regular
   * 1 = log
   * 2 = exp
   */
  virtual std::vector<std::vector<unsigned int>> getTransform() const = 0;

  /// Returns factors for the functions for the conversion functions given in getTransform
  virtual std::vector<std::vector<Real>> getTransformCoefs() const = 0;

  /* Returns human-readable limits for the inputs. Inputs ordering is
   * 0: mobile
   * 1: immobile_old
   * 2: trial stress,
   * 3: effective strain old,
   * 4: temperature
   * 5: environmental factor (optional)
   */
  virtual std::vector<std::vector<Real>> getInputLimits() const = 0;

  /// Material specific coefficients multiplied by the Legendre polynomials for each of the input variables
  virtual std::vector<std::vector<Real>> getCoefs() const = 0;
};
