//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTensorAverage.h"

registerMooseObject("TensorMechanicsApp", MaterialTensorAverage);
registerMooseObject("TensorMechanicsApp", ADMaterialTensorAverage);

template <bool is_ad>
InputParameters
MaterialTensorAverageTempl<is_ad>::validParams()
{
  InputParameters params = MaterialTensorIntegralTempl<is_ad>::validParams();
  params.addClassDescription("Computes the average of a RankTwoTensor component over a volume.");
  return params;
}

template <bool is_ad>
MaterialTensorAverageTempl<is_ad>::MaterialTensorAverageTempl(const InputParameters & parameters)
  : MaterialTensorIntegralTempl<is_ad>(parameters), _volume(0.0)
{
}

template <bool is_ad>
void
MaterialTensorAverageTempl<is_ad>::initialize()
{
  MaterialTensorIntegralTempl<is_ad>::initialize();

  _volume = 0.0;
}

template <bool is_ad>
void
MaterialTensorAverageTempl<is_ad>::execute()
{
  MaterialTensorIntegralTempl<is_ad>::execute();

  _volume += this->_current_elem_volume;
}

template <bool is_ad>
Real
MaterialTensorAverageTempl<is_ad>::getValue()
{
  return _integral_value / _volume;
}

template <bool is_ad>
void
MaterialTensorAverageTempl<is_ad>::finalize()
{
  MaterialTensorIntegralTempl<is_ad>::gatherSum(_volume);
  MaterialTensorIntegralTempl<is_ad>::gatherSum(_integral_value);
}

template <bool is_ad>
void
MaterialTensorAverageTempl<is_ad>::threadJoin(const UserObject & y)
{
  MaterialTensorIntegralTempl<is_ad>::threadJoin(y);

  const MaterialTensorAverageTempl<is_ad> & pps =
      static_cast<const MaterialTensorAverageTempl<is_ad> &>(y);
  _volume += pps._volume;
}

template class MaterialTensorAverageTempl<false>;
template class MaterialTensorAverageTempl<true>;
