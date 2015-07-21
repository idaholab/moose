#include "HyperElasticStress.h"

template<>
InputParameters validParams<HyperElasticStress>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

HyperElasticStress::HyperElasticStress(const InputParameters & parameters) :
    GeneralUserObject(parameters)
{
  //Sets number of states used and computed in this user object
  _num_in_state = 0;
  _num_out_state_real = 0;
  _num_out_state_ranktwotensor = 0;
}

unsigned int
HyperElasticStress::getNumStateIn() const
{
  return _num_in_state;
}

unsigned int
HyperElasticStress::getNumStateOutReal() const
{
  return _num_out_state_real;
}

unsigned int
HyperElasticStress::getNumStateOutRankTwoTensor() const
{
  return _num_out_state_ranktwotensor;
}

RankFourTensor
HyperElasticStress::computeDstrainDce(const RankTwoTensor & ce) const
{
  RankFourTensor val;

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
      val(i, j, i, j) = 0.5;

  return val;
}

void
HyperElasticStress::computePK2Stress(RankTwoTensor & pk2, RankFourTensor &dpk2_dce, std::vector<Real> & out_state_real, std::vector<RankTwoTensor> & out_state_ranktwotensor,const RankTwoTensor & ce, const RankFourTensor & elasticity_tensor, const std::vector<Real> & in_state) const
{
  RankTwoTensor ee = computeStrain(ce);
  pk2 = elasticity_tensor * ee;
  dpk2_dce = elasticity_tensor * computeDstrainDce(ce);
}

RankTwoTensor
HyperElasticStress::computeStrain(const RankTwoTensor & ce) const
{
  RankTwoTensor iden;
  iden.addIa(1.0);
  return 0.5 * (ce-iden);
}

// DEPRECATED
HyperElasticStress::HyperElasticStress(const std::string & name,
                                       InputParameters parameters) :
    GeneralUserObject(name, parameters)
{
  //Sets number of states used and computed in this user object
  _num_in_state = 0;
  _num_out_state_real = 0;
  _num_out_state_ranktwotensor = 0;
}
