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
#ifndef AUXCOUPLEDMATERIAL_H
#define AUXCOUPLEDMATERIAL_H

#include "Material.h"

class AuxCoupledMaterial;

template<>
InputParameters validParams<AuxCoupledMaterial>();

/**
 * A material that couples a variable
 */
class AuxCoupledMaterial : public Material
{
public:
  AuxCoupledMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();
  const VariableValue & _variable;
  MaterialProperty<Real> & _mat_prop;
  MaterialProperty<Real> & _mat_prop_old;
};

#endif // AUXCOUPLEDMATERIAL_H
