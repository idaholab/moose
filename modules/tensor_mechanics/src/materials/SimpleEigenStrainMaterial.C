// Original class author: A.M. Jokisaari, O. Heinonen

#include "SimpleEigenStrainMaterial.h"

/**
 * SimpleEigenStrainMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<SimpleEigenStrainMaterial>()
{
  InputParameters params = validParams<EigenStrainBaseMaterial>();
  params.addRequiredParam<Real>("epsilon0","Initial eigen strain value");
  params.addParam<Real>("c0",0,"Initial concentration value");

  return params;
}

SimpleEigenStrainMaterial::SimpleEigenStrainMaterial(const std::string & name,
                                             InputParameters parameters)
    : EigenStrainBaseMaterial(name, parameters),
      _epsilon0(getParam<Real>("epsilon0")),
      _c0(getParam<Real>("c0"))
{
}

void SimpleEigenStrainMaterial::computeEigenStrain()
{

  RankTwoTensor init_eigenstrain, init_deigenstrain_dc;
  init_eigenstrain.addIa(_epsilon0*(_c[_qp] - _c0));

  _eigenstrain[_qp] = init_eigenstrain;
  _deigenstrain_dc[_qp] = init_deigenstrain_dc;
  _d2eigenstrain_dc2[_qp].zero();
}

void SimpleEigenStrainMaterial::computeQpElasticityTensor()
{

  _elasticity_tensor[_qp] = _Cijkl;
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

  _delasticity_tensor_dc[_qp].zero();

  _d2elasticity_tensor_dc2[_qp].zero();

}
