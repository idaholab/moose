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
#ifndef INCREMENT_MATERIAL_H
#define INCREMENT_MATERIAL_H

#include "GenericConstantMaterial.h"

class IncrementMaterial;

template <>
InputParameters validParams<GenericConstantMaterial>();

/**
 * A material that tracks the number of times computeQpProperties has been called.
 */
class IncrementMaterial : public GenericConstantMaterial
{
public:
  IncrementMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  unsigned int _inc;
  MaterialProperty<Real> & _mat_prop;
};

#endif
