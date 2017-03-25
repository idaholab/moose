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
#ifndef STATEFULSPATIALTEST_H
#define STATEFULSPATIALTEST_H

#include "Material.h"

// Forward Declarations
class StatefulSpatialTest;

template <>
InputParameters validParams<StatefulSpatialTest>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class StatefulSpatialTest : public Material
{
public:
  StatefulSpatialTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  MaterialProperty<Real> & _thermal_conductivity;
  const MaterialProperty<Real> & _thermal_conductivity_old;
};

#endif // STATEFULSPATIALTEST_H
