//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"
#include "LayeredBase.h"

/**
 * Base class for computing layered side averages
 */
template <typename BaseType>
class LayeredSideAverageBase : public BaseType
{
public:
  static InputParameters validParams();

  LayeredSideAverageBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  using BaseType::gatherSum;
  using BaseType::getLayer;
  using BaseType::getLayerValue;

protected:
  /// Value of the volume for each layer
  std::vector<Real> _layer_volumes;

  using BaseType::_current_elem;
  using BaseType::_current_side_volume;
  using BaseType::_num_layers;
  using BaseType::layerHasValue;
  using BaseType::setLayerValue;
};

template <typename BaseType>
InputParameters
LayeredSideAverageBase<BaseType>::validParams()
{
  InputParameters params = BaseType::validParams();
  return params;
}

template <typename BaseType>
LayeredSideAverageBase<BaseType>::LayeredSideAverageBase(const InputParameters & parameters)
  : BaseType(parameters)
{
  _layer_volumes.resize(_num_layers);
}

template <typename BaseType>
void
LayeredSideAverageBase<BaseType>::initialize()
{
  BaseType::initialize();

  for (auto & vol : _layer_volumes)
    vol = 0.0;
}

template <typename BaseType>
void
LayeredSideAverageBase<BaseType>::execute()
{
  BaseType::execute();

  const auto layer = getLayer(_current_elem->vertex_average());
  _layer_volumes[layer] += _current_side_volume;
}

template <typename BaseType>
void
LayeredSideAverageBase<BaseType>::finalize()
{
  BaseType::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for (const auto i : index_range(_layer_volumes))
    if (layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

template <typename BaseType>
void
LayeredSideAverageBase<BaseType>::threadJoin(const UserObject & y)
{
  BaseType::threadJoin(y);
  const auto & lsa = static_cast<const LayeredSideAverageBase<BaseType> &>(y);
  for (const auto i : index_range(_layer_volumes))
    if (lsa.layerHasValue(i))
      _layer_volumes[i] += lsa._layer_volumes[i];
}
