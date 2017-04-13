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

#include "Material.h"

#ifndef BADSTATEFULMATERIAL_H
#define BADSTATEFULMATERIAL_H

// Forward Declarations
class BadStatefulMaterial;

template <>
InputParameters validParams<BadStatefulMaterial>();

/// Tries to retrieve non-existing old/older versions of a material property.
class BadStatefulMaterial : public Material
{
public:
  BadStatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif // STATEFULMATERIAL_H
