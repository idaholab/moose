/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#include "GenericMaterialA.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseObject("SolidMechanicsApp", GenericMaterialA);
registerMooseObject("SolidMechanicsApp", ADGenericMaterialA);

template <bool is_ad>
InputParameters
GenericMaterialATempl<is_ad>::validParams()
{
  InputParameters params = ComputeEigenstrainBaseTempl<is_ad>::validParams();
  params.addClassDescription("Generic material A for debug of activiting blocks.");
  params.suppressParameter<std::string>("base_name");
  params.set<std::string>("eigenstrain_name") = "adjustment_eigenstrain";
  params.suppressParameter<std::string>("eigenstrain_name");
  params.makeParamRequired<std::vector<SubdomainName>>("block");

  return params;
}

template <bool is_ad>
GenericMaterialATempl<is_ad>::GenericMaterialATempl(const InputParameters & parameters)
  : ComputeEigenstrainBaseTempl<is_ad>(parameters),
    _eigenstrain_old(this->template getMaterialPropertyOld<RankTwoTensor>(
        this->template getParam<std::string>("eigenstrain_name"))),
    _mechanical_strain(
        this->template getGenericMaterialProperty<RankTwoTensor, is_ad>("mechanical_strain")),
    _total_strain(this->template getGenericMaterialProperty<RankTwoTensor, is_ad>("total_strain")),
    _elastic_strain(
        this->template getGenericMaterialProperty<RankTwoTensor, is_ad>("elastic_strain"))
{
}
template <bool is_ad>
void
GenericMaterialATempl<is_ad>::initQpStatefulProperties()
{
  _eigenstrain[_qp].zero();
}

template <bool is_ad>
void
GenericMaterialATempl<is_ad>::computeQpEigenstrain()
{
  _eigenstrain[_qp] = -_mechanical_strain[_qp];
}

template class GenericMaterialATempl<false>;
template class GenericMaterialATempl<true>;
