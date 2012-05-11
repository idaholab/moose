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

#ifndef STATEFULMATERIAL_H
#define STATEFULMATERIAL_H

//Forward Declarations
class StatefulMaterial;

template<>
InputParameters validParams<StatefulMaterial>();

/**
 * Stateful material class that defines a few properties.
 */
class StatefulMaterial : public Material
{
public:
  StatefulMaterial(const std::string & name,
                  InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

private:
  Real _initial_diffusivity;

  /**
   * Create two MooseArray Refs to hold the current
   * and previous material properties respectively
   */
  MaterialProperty<Real> & _diffusivity;
  MaterialProperty<Real> & _diffusivity_old;
};

#endif //STATEFULMATERIAL_H
