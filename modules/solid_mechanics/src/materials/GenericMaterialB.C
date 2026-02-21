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

#include "GenericMaterialB.h"

registerMooseObject("SolidMechanicsApp", GenericMaterialB);
registerMooseObject("SolidMechanicsApp", ADGenericMaterialB);

template <bool is_ad>
InputParameters
GenericMaterialBTempl<is_ad>::validParams()
{
  InputParameters params = ComputeEigenstrainBaseTempl<is_ad>::validParams();
  params.addClassDescription("Generic material B for debug of activiting blocks.");
  params.suppressParameter<std::string>("base_name");
  params.set<std::string>("eigenstrain_name") = "adjustment_eigenstrain";
  params.suppressParameter<std::string>("eigenstrain_name");
  params.makeParamRequired<std::vector<SubdomainName>>("block");

  return params;
}

template <bool is_ad>
GenericMaterialBTempl<is_ad>::GenericMaterialBTempl(const InputParameters & parameters)
  : ComputeEigenstrainBaseTempl<is_ad>(parameters),
    _eigenstrain_old(this->template getMaterialPropertyOld<RankTwoTensor>(
        this->template getParam<std::string>("eigenstrain_name")))
{
}

template <bool is_ad>
void
GenericMaterialBTempl<is_ad>::initQpStatefulProperties()
{
}

template <bool is_ad>
void
GenericMaterialBTempl<is_ad>::computeQpEigenstrain()
{
  _eigenstrain[_qp] = _eigenstrain_old[_qp];
}

template class GenericMaterialBTempl<false>;
template class GenericMaterialBTempl<true>;
