//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

/**
 * Base class for defining solid property materials.
 */
class SolidPropertiesMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  SolidPropertiesMaterial(const InputParameters & parameters);

  /**
   * Solid name
   * @return string representing solid name
   */
  virtual const std::string & solidName() const;

  /**
   * Molar mass
   * @return molar mass
   */
  virtual Real molarMass() const;

  /**
   * Isobaric specific heat capacity
   */
  virtual void computeIsobaricSpecificHeat();

  /**
   * Isobaric specific heat capacity derivatives
   */
  virtual void computeIsobaricSpecificHeatDerivatives();

  /**
   * Thermal conductivity
   */
  virtual void computeThermalConductivity();

  /**
   * Thermal conductivity derivatives
   */
  virtual void computeThermalConductivityDerivatives();

  /**
   * Density
   */
  virtual void computeDensity();

  /**
   * Density derivatives
   */
  virtual void computeDensityDerivatives();

  /**
   * Thermal expansion coefficient
   */
  virtual void computeThermalExpansionCoefficient();

  /**
   * Surface emissivity
   */
  virtual void computeSurfaceEmissivity();

private:
  /// The solid name
  static const std::string _name;
};
