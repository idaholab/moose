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
#ifndef RECOMPUTEMATERIAL_H
#define RECOMPUTEMATERIAL_H

#include "Material.h"

class RecomputeMaterial;

template<>
InputParameters validParams<RecomputeMaterial>();

/**
 * A test material for testing the ability for properties to be recomputed
 *
 * @see NewtonMaterial
 */
class RecomputeMaterial : public Material
{
public:
  RecomputeMaterial(const std::string & name, InputParameters parameters);
  virtual ~RecomputeMaterial(){};

protected:
  void computeQpProperties();

private:
  VariableValue & _var;
  MaterialProperty<Real> & _f;
  MaterialProperty<Real> &_f_prime;
  const MaterialProperty<Real> & _p;
  MaterialProperty<Real> & _dummy;
};

#endif /* RECOMPUTEMATERIAL_H */
