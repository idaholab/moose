// Original class author: A.M. Jokisaari, O. Heinonen

#include "TensorElasticMaterial.h"

/**
 * TensorElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<TensorElasticMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();

  return params;
}

TensorElasticMaterial::TensorElasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters)
{
}

void TensorElasticMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
