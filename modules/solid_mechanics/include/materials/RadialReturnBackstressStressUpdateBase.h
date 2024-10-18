#pragma once

#include "RadialReturnStressUpdate.h"

template <bool is_ad>
class RadialReturnBackstressStressUpdateBaseTempl : public RadialReturnStressUpdateTempl<is_ad>
{
public:
  static InputParameters validParams();

  RadialReturnBackstressStressUpdateBaseTempl(const InputParameters & parameters);

  using Material::_qp;
  using RadialReturnStressUpdateTempl<is_ad>::propagateQpStatefulPropertiesRadialReturn;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void updateState(GenericRankTwoTensor<is_ad> & strain_increment,
                           GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                           const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
                           GenericRankTwoTensor<is_ad> & stress_new,
                           const RankTwoTensor & /*stress_old*/,
                           const GenericRankFourTensor<is_ad> & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;

  GenericMaterialProperty<RankTwoTensor, is_ad> & _backstress;
  const MaterialProperty<RankTwoTensor> & _backstress_old;
};

typedef RadialReturnBackstressStressUpdateBaseTempl<false> RadialReturnBackstressStressUpdateBase;
typedef RadialReturnBackstressStressUpdateBaseTempl<true> ADRadialReturnBackstressStressUpdateBase;
