//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredIntegralBase.h"

/**
 * Base class for computing layered averages
 */
template <typename BaseType>
class LayeredAverageBase : public LayeredIntegralBase<BaseType>
{
public:
  static InputParameters validParams();

  LayeredAverageBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  using LayeredIntegralBase<BaseType>::gatherSum;
  using LayeredIntegralBase<BaseType>::getLayer;
  using LayeredIntegralBase<BaseType>::getLayerValue;

protected:
  /**
   * @returns The local integration volume (or area in the case of a side integral). This is not the
   * layer volume
   */
  virtual Real volume() const = 0;

  /// Value of the volume (area for side integrals) for each layer
  std::vector<Real> _layer_volumes;

  using LayeredIntegralBase<BaseType>::_current_elem;
  using LayeredIntegralBase<BaseType>::_num_layers;
  using LayeredIntegralBase<BaseType>::layerHasValue;
  using LayeredIntegralBase<BaseType>::setLayerValue;
};

template <typename BaseType>
InputParameters
LayeredAverageBase<BaseType>::validParams()
{
  InputParameters params = LayeredIntegralBase<BaseType>::validParams();
  return params;
}

template <typename BaseType>
LayeredAverageBase<BaseType>::LayeredAverageBase(const InputParameters & parameters)
  : LayeredIntegralBase<BaseType>(parameters)
{
  _layer_volumes.resize(_num_layers);
}

template <typename BaseType>
void
LayeredAverageBase<BaseType>::initialize()
{
  LayeredIntegralBase<BaseType>::initialize();

  for (auto & vol : _layer_volumes)
    vol = 0.0;
}

template <typename BaseType>
void
LayeredAverageBase<BaseType>::execute()
{
  LayeredIntegralBase<BaseType>::execute();

  const auto layer = getLayer(_current_elem->vertex_average());
  _layer_volumes[layer] += volume();
}

template <typename BaseType>
void
LayeredAverageBase<BaseType>::finalize()
{
  LayeredIntegralBase<BaseType>::finalize();

  gatherSum(_layer_volumes);

  // Compute the average for each layer
  for (const auto i : index_range(_layer_volumes))
    if (layerHasValue(i))
      setLayerValue(i, getLayerValue(i) / _layer_volumes[i]);
}

template <typename BaseType>
void
LayeredAverageBase<BaseType>::threadJoin(const UserObject & y)
{
  LayeredIntegralBase<BaseType>::threadJoin(y);
  const auto & lsa = static_cast<const LayeredAverageBase<BaseType> &>(y);
  for (const auto i : index_range(_layer_volumes))
    if (lsa.layerHasValue(i))
      _layer_volumes[i] += lsa._layer_volumes[i];
}
