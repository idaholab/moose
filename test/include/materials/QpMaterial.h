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
#ifndef QPMATERIAL_H
#define QPMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"

// Forward Declarations
class QpMaterial;

template <>
InputParameters validParams<QpMaterial>();

/**
 * Material with a single property that corresponds to the quadrature
 * point index.  Used to ensure that the constant_on_elem flag
 * actually works correctly (the MaterialProperty should output as
 * identically zero when the flag is turned on).
 */
class QpMaterial : public Material
{
public:
  QpMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const std::string _prop_name;
  MaterialProperty<Real> & _mat_prop;
};

#endif
