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
#ifndef ADCOUPLEDMATERIAL_H_
#define ADCOUPLEDMATERIAL_H_

#include "Material.h"

class ADCoupledMaterial;

template<>
InputParameters validParams<ADCoupledMaterial>();

/**
 * A material that couples a material property
 */
class ADCoupledMaterial : public Material
{
public:
  ADCoupledMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  std::string _mat_prop_name;
  MaterialProperty<ADReal> & _mat_prop;

  ADVariableValue & _coupled_var;

  //const MaterialProperty<ADReal> & _coupled_mat_prop;
};

#endif //ADCOUPLEDMATERIAL_H
