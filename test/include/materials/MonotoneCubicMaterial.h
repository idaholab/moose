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
#ifndef MONOTONECUBICMATERIAL_H
#define MONOTONECUBICMATERIAL_H

#include "Material.h"

#include "MonotoneCubicInterpolation.h"

class MonotoneCubicMaterial;

template<>
InputParameters validParams<MonotoneCubicMaterial>();

class MonotoneCubicMaterial : public Material
{
public:
  MonotoneCubicMaterial(const InputParameters & parameters);

  virtual ~MonotoneCubicMaterial() = default;

protected:
  virtual void computeQpProperties();

  MonotoneCubicInterpolation _monotone_interp;
  MaterialProperty<Real> & _property;
};

#endif //MONOTONECUBICMATERIAL_H
