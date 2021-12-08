//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialADConverter.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", MaterialADConverter);
registerMooseObject("MooseApp", RankFourTensorMaterialADConverter);
registerMooseObject("MooseApp", RankTwoTensorMaterialADConverter);
registerMooseObjectRenamed("MooseApp", MaterialConverter, "06/30/2022 24:00", MaterialADConverter);
registerMooseObjectRenamed("MooseApp",
                           RankTwoTensorMaterialConverter,
                           "06/30/2022 24:00",
                           RankTwoTensorMaterialADConverter);
registerMooseObjectRenamed("MooseApp",
                           RankFourTensorMaterialConverter,
                           "06/30/2022 24:00",
                           RankFourTensorMaterialADConverter);

template <typename T>
InputParameters
MaterialADConverterTempl<T>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Converts regular material properties to AD properties and vice versa");
  params.addParam<std::vector<MaterialPropertyName>>(
      "reg_props_in", "The names of the regular material properties to convert to AD properties");
  params.addParam<std::vector<MaterialPropertyName>>("ad_props_out",
                                                     "The names of the output AD properties");
  params.addParam<std::vector<MaterialPropertyName>>(
      "ad_props_in", "The names of the AD material properties to convert to regular properties");
  params.addParam<std::vector<MaterialPropertyName>>("reg_props_out",
                                                     "The names of the output regular properties");
  params.addParam<bool>(
      "intra_convert", false, "Whether to allow intra conversion, e.g. regular->regular, ad->ad");
  return params;
}

template <typename T>
MaterialADConverterTempl<T>::MaterialADConverterTempl(const InputParameters & parameters)
  : Material(parameters), _intra_convert(getParam<bool>("intra_convert"))
{
  auto reg_props_in = getParam<std::vector<MaterialPropertyName>>("reg_props_in");
  auto ad_props_out = getParam<std::vector<MaterialPropertyName>>("ad_props_out");
  auto ad_props_in = getParam<std::vector<MaterialPropertyName>>("ad_props_in");
  auto reg_props_out = getParam<std::vector<MaterialPropertyName>>("reg_props_out");

  if (_intra_convert)
  {
    if (reg_props_in.size() != reg_props_out.size())
      paramError("reg_props_out",
                 "The number of output regular properties must match the number of input regular "
                 "properties, which is " +
                     std::to_string(reg_props_in.size()));
  }
  else
  {
    if (reg_props_in.size() != ad_props_out.size())
      paramError("ad_props_out",
                 "The number of output AD properties must match the number of input regular "
                 "properties, which is " +
                     std::to_string(reg_props_in.size()));
  }

  _num_reg_props_to_convert = reg_props_in.size();

  if (_intra_convert)
  {
    if (ad_props_in.size() != ad_props_out.size())
      paramError("ad_props_out",
                 "The number of output AD properties must match the number of input AD "
                 "properties, which is " +
                     std::to_string(ad_props_in.size()));
  }
  else
  {
    if (ad_props_in.size() != reg_props_out.size())
      paramError("reg_props_out",
                 "The number of output regular properties must match the number of input AD "
                 "properties, which is " +
                     std::to_string(ad_props_in.size()));
  }

  _num_ad_props_to_convert = ad_props_in.size();

  _reg_props_in.resize(_num_reg_props_to_convert);
  _ad_props_out.resize(ad_props_out.size());
  _ad_props_in.resize(_num_ad_props_to_convert);
  _reg_props_out.resize(reg_props_out.size());

  for (MooseIndex(_num_reg_props_to_convert) i = 0; i < _num_reg_props_to_convert; ++i)
    _reg_props_in[i] = &getMaterialProperty<T>(reg_props_in[i]);

  for (MooseIndex(ad_props_out) i = 0; i < ad_props_out.size(); ++i)
    _ad_props_out[i] = &declareADProperty<T>(ad_props_out[i]);

  for (MooseIndex(_num_ad_props_to_convert) i = 0; i < _num_ad_props_to_convert; ++i)
    _ad_props_in[i] = &getADMaterialProperty<T>(ad_props_in[i]);

  for (MooseIndex(reg_props_out) i = 0; i < reg_props_out.size(); ++i)
    _reg_props_out[i] = &declareProperty<T>(reg_props_out[i]);
}

template <typename T>
void
MaterialADConverterTempl<T>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <typename T>
void
MaterialADConverterTempl<T>::computeQpProperties()
{
  if (_intra_convert)
    for (MooseIndex(_num_reg_props_to_convert) i = 0; i < _num_reg_props_to_convert; ++i)
      (*_reg_props_out[i])[_qp] = (*_reg_props_in[i])[_qp];
  else
    for (MooseIndex(_num_reg_props_to_convert) i = 0; i < _num_reg_props_to_convert; ++i)
      (*_ad_props_out[i])[_qp] = (*_reg_props_in[i])[_qp];

  if (_intra_convert)
    for (MooseIndex(_num_ad_props_to_convert) i = 0; i < _num_ad_props_to_convert; ++i)
      (*_ad_props_out[i])[_qp] = (*_ad_props_in[i])[_qp];
  else
    for (MooseIndex(_num_ad_props_to_convert) i = 0; i < _num_ad_props_to_convert; ++i)
      (*_reg_props_out[i])[_qp] = MetaPhysicL::raw_value((*_ad_props_in[i])[_qp]);
}

template class MaterialADConverterTempl<Real>;
template class MaterialADConverterTempl<RankFourTensor>;
template class MaterialADConverterTempl<RankTwoTensor>;
