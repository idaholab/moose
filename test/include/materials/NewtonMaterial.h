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
#ifndef NEWTONMATERIAL_H
#define NEWTONMATERIAL_H

#include "Material.h"

class NewtonMaterial;

template<>
InputParameters validParams<NewtonMaterial>();

/**
 * Adds two material properties together
 */
class NewtonMaterial : public Material
{
public:
  NewtonMaterial(const std::string & name, InputParameters parameters);
  virtual ~NewtonMaterial(){};

protected:
  void computeQpProperties();

private:
  const Real & _tol;
  const MaterialProperty<Real> & _f;
  const MaterialProperty<Real> &_f_prime;
  MaterialProperty<Real> & _p;
  std::vector<unsigned int> _prop_ids;
};

#endif /* NEWTONMATERIAL_H */
