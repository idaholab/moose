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
#ifndef SUMMATERIAL_H
#define SUMMATERIAL_H

#include "Material.h"

class SumMaterial;

template<>
InputParameters validParams<SumMaterial>();

/**
 * Adds two material properties together
 */
class SumMaterial : public Material
{
public:
  SumMaterial(const std::string & name, InputParameters parameters);
  virtual ~SumMaterial();

protected:
  void computeQpProperties();

  std::string _sum_prop_name;
  std::string _mp1_prop_name;
  std::string _mp2_prop_name;

  MaterialProperty<Real> & _sum;
  MaterialProperty<Real> & _mp1;
  MaterialProperty<Real> & _mp2;

  Real _val_mp1;
  Real _val_mp2;
};

#endif /* SUMMATERIAL_H */
