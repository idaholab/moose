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

// MOOSE includes
#include "Material.h"

// Forward declarations
class NewtonMaterial;
class Material;

template <>
InputParameters validParams<NewtonMaterial>();

/**
 * A test object that uses Material to perform a Newton solve of a material property.
 *
 * Also, does some error checking.
 */
class NewtonMaterial : public Material
{
public:
  NewtonMaterial(const InputParameters & parameters);
  virtual ~NewtonMaterial(){};

protected:
  void computeQpProperties();

private:
  const Real & _tol;
  const MaterialProperty<Real> & _f;
  const MaterialProperty<Real> & _f_prime;
  MaterialProperty<Real> & _p;
  std::vector<unsigned int> _prop_ids;
  unsigned int _max_iterations;
  Material & _discrete;
};

#endif /* NEWTONMATERIAL_H */
