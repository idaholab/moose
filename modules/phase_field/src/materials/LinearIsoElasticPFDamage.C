/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearIsoElasticPFDamage.h"

template<>
InputParameters validParams<LinearIsoElasticPFDamage>()
{
  InputParameters params = validParams<LinearElasticMaterial>();
  params.addClassDescription("Phase-field fracture model energy contribution to damage growth");
  params.addRequiredCoupledVar("c","Order parameter for damage");
  params.addParam<Real>("kdamage",1e-6,"Stiffness of damaged matrix");
  params.addClassDescription("Strain energy density in damaged isotropic elastic material - undamaged stress under compressive strain");

  return params;
}

LinearIsoElasticPFDamage::LinearIsoElasticPFDamage(const std::string & name,
                                             InputParameters parameters) :
  LinearElasticMaterial(name, parameters),
  _c(coupledValue("c")),
  _kdamage(getParam<Real>("kdamage")),
  _dstress_dc(declareProperty<RankTwoTensor>("dstress_dc")),
  _G0_pos(declareProperty<Real>("G0_pos")),
  _dG0_pos_dstrain(declareProperty<RankTwoTensor>("dG0_pos_dstrain"))
{
  _etens.resize(LIBMESH_DIM);
  _epos.resize(LIBMESH_DIM);
  _scaling = 1.0;
}

void LinearIsoElasticPFDamage::computeQpStress()
{
  _stress[_qp].zero();
  updateVar();
}

void
LinearIsoElasticPFDamage::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Cijkl;

  getScalingFactor();

  if ( _scaling < 0.0 )
    mooseError("LinearIsoElasticPFDamage: scaling parameter cannot be negative");
  /**
   * Avoids zeroing of elasticity tensor if scaling is 0.0 (void with zero stiffness)
   * Adds user specified small number kdamage
   */
  _scaling += _kdamage;

  _elasticity_tensor[_qp] *= _scaling;
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
/**
 * This function obtains scaling parameter for elasticity tensor
 * Can happen because of heterogenities such as void or particle
 * User should override this function if such heterogenities needs consideration
 */
void
LinearIsoElasticPFDamage::getScalingFactor()
{
  _scaling = 1.0;
}

void
LinearIsoElasticPFDamage::updateVar()
{
  RankTwoTensor stress0pos,stress0neg,stress0;

  //Isotropic elasticity is assumed
  Real lambda = _elasticity_tensor[_qp](0,0,1,1);
  Real mu = _elasticity_tensor[_qp](0,1,0,1);

  std::vector<Real> w;
  RankTwoTensor evec;

  _elastic_strain[_qp].symmetricEigenvaluesEigenvectors(w,evec);

  //Tensors of outerproduct of eigen vectors
  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    for ( unsigned int j = 0; j < LIBMESH_DIM; j++ )
      for ( unsigned int k = 0; k < LIBMESH_DIM; k++ )
        _etens[i](j,k) = evec(j,i) * evec(k,i);

  Real etr=0.0;
  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    etr += w[i];

  Real etrpos=(std::abs(etr)+etr)/2.0;
  Real etrneg=(std::abs(etr)-etr)/2.0;

  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
  {
    stress0pos += _etens[i] * ( lambda * etrpos + 2.0 * mu * (std::abs(w[i]) + w[i])/2.0);
    stress0neg += _etens[i] * ( lambda * etrneg + 2.0 * mu * (std::abs(w[i]) - w[i])/2.0);
  }

  Real c = _c[_qp];
  Real xfac = std::pow(1.0-c,2.0) + _kdamage;

  //Damage associated with positive component of stress
  _stress[_qp] = stress0pos * xfac - stress0neg;

  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    _epos[i] = (std::abs(w[i]) + w[i])/2.0;

  Real val = 0.0;
  for (unsigned int i = 0; i < LIBMESH_DIM; i++ )
    val += std::pow(_epos[i],2.0);
  val *= mu;

  //Energy with positive principal strains
  _G0_pos[_qp] = lambda * std::pow(etrpos,2.0)/2.0 + val;

  //Used in PFFracBulkRate Jacobian
  _dG0_pos_dstrain[_qp] = stress0pos;
  //Used in StressDivergencePFFracTensors Jacobian
  _dstress_dc[_qp] = -stress0pos * (2 * (1.0-c));
}
