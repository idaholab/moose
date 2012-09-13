// Original class author: A.M. Jokisaari, O. Heinonen

#include "LinearElasticMaterial.h"

/**
 * LinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<LinearElasticMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();

  return params;
}

LinearElasticMaterial::LinearElasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters)
{
}

void LinearElasticMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
