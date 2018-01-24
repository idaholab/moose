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
#ifndef COUPLEDMATERIAL2_H_
#define COUPLEDMATERIAL2_H_

#include "Material.h"

class CoupledMaterial2;

template <>
InputParameters validParams<CoupledMaterial2>();

/**
 * A material that couples a material property
 */
class CoupledMaterial2 : public Material
{
public:
  CoupledMaterial2(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialPropertyName _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  const MaterialProperty<Real> & _coupled_mat_prop;
  const MaterialProperty<Real> & _coupled_mat_prop2;
};

#endif // COUPLEDMATERIAL2_H
