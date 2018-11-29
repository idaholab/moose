//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLIDPROPERTIESMATERIAL_H
#define SOLIDPROPERTIESMATERIAL_H

#include "Material.h"

class SolidPropertiesMaterial;

template <>
InputParameters validParams<SolidPropertiesMaterial>();

/**
 * Base class for defining solid property materials.
 */
class SolidPropertiesMaterial : public Material
{
public:
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
   * @return isobaric specific heat capacity
   */
  virtual Real cp() const;

  /**
   * Compute thermal conductivity
   * @return thermal conductivity
   */
  virtual Real k() const;

  /**
   * Compute density
   * @return density
   */
  virtual Real rho() const;

  /**
   * Thermal expansion coefficient
   * @return thermal expansion coefficient
   */
  virtual Real beta() const;

  /**
   * Surface emissivity
   * @return surface emissivity
   */
  virtual Real surface_emissivity() const;

private:
  /// The solid name
  static const std::string _name;
};

#endif /* SOLIDPROPERTIESMATERIAL_H */
