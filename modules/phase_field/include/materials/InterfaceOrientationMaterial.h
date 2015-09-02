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

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

#include "Material.h"

//Forward Declarations
class ExampleMaterial;

template<>
InputParameters validParams<ExampleMaterial>();

/**
 * Example material class that defines a few properties.
 */
class ExampleMaterial : public Material
{
public:
  ExampleMaterial(const std::string & name,
                  InputParameters parameters);

protected:
  virtual void computeProperties();

private:
  MaterialProperty<Real> & _eps;
  MaterialProperty<Real> & _eps1;
  VariableValue & _u;
  VariableGradient & _grad_u;
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;
  MaterialProperty<Real> & _kappa_c;
  Real _mob;
  Real _kappa;

};

#endif //EXAMPLEMATERIAL_H
