#include "RadialReturnBackstressStressUpdateBase.h"
#include "RankTwoTensorForward.h"

template <bool is_ad>
InputParameters
RadialReturnBackstressStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnStressUpdateTempl<is_ad>::validParams();
  return params;
}

template <bool is_ad>
RadialReturnBackstressStressUpdateBaseTempl<is_ad>::RadialReturnBackstressStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<is_ad>(parameters),
    _backstress(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                            "backstress")),
    _backstress_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "backstress"))
{
}

template <bool is_ad>
void
RadialReturnBackstressStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _backstress[_qp].zero();
  RadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
RadialReturnBackstressStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _backstress[_qp] = _backstress_old[_qp];
  propagateQpStatefulPropertiesRadialReturn();
}

template <bool is_ad>
void
RadialReturnBackstressStressUpdateBaseTempl<is_ad>::updateState(
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & rotation_increment,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  GenericRankTwoTensor<is_ad> stress_corrected = stress_new - _backstress_old[_qp];

  RadialReturnStressUpdateTempl<is_ad>::updateState(strain_increment,
                                                    inelastic_strain_increment,
                                                    rotation_increment,
                                                    stress_corrected,
                                                    stress_old,
                                                    elasticity_tensor,
                                                    elastic_strain_old,
                                                    compute_full_tangent_operator,
                                                    tangent_operator);
  stress_new = stress_corrected;
}

template class RadialReturnBackstressStressUpdateBaseTempl<false>;
template class RadialReturnBackstressStressUpdateBaseTempl<true>;
