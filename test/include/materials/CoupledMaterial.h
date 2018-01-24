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
#ifndef COUPLEDMATERIAL_H_
#define COUPLEDMATERIAL_H_

#include "Material.h"

class CoupledMaterial;

template <>
InputParameters validParams<CoupledMaterial>();

/**
 * A material that couples a material property
 */
class CoupledMaterial : public Material
{
public:
  CoupledMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override { _mat_prop[_qp] = 1.0; }
  virtual void computeQpProperties() override;

  std::string _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  const MaterialProperty<Real> & _coupled_mat_prop;
};

#endif // COUPLEDMATERIAL_H
