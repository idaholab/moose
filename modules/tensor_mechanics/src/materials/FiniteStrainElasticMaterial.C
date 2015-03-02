/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari, O. Heinonen

#include "FiniteStrainElasticMaterial.h"

/**
 * FiniteStrainElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<FiniteStrainElasticMaterial>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();
  return params;
}

FiniteStrainElasticMaterial::FiniteStrainElasticMaterial(const std::string & name,
                                             InputParameters parameters) :
    FiniteStrainMaterial(name, parameters)
{
}

void FiniteStrainElasticMaterial::computeQpStress()
{
  //Update strain in intermediate configuration
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  //Rotate strain to current configuration
  _total_strain[_qp] = _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();

  //For elastic problems elastic strain = total strain
  _elastic_strain[_qp]=_total_strain[_qp];

  // stress = C * e
  _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp]; //Calculate stress in intermediate configruation

  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();

}
