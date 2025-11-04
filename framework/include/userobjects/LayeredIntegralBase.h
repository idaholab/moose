//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpatialUserObjectFunctor.h"
#include "LayeredBase.h"
#include "InputParameters.h"

/**
 * Base class for computing layered side integrals
 */
template <typename UserObjectType>
class LayeredIntegralBase : public SpatialUserObjectFunctor<UserObjectType>, public LayeredBase
{
public:
  static InputParameters validParams();

  LayeredIntegralBase(const InputParameters & parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const override { return integralValue(p); }

  virtual const std::vector<Point> spatialPoints() const override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  using UserObjectType::_current_elem;
  using UserObjectType::computeIntegral;
};

template <typename UserObjectType>
InputParameters
LayeredIntegralBase<UserObjectType>::validParams()
{
  InputParameters params = SpatialUserObjectFunctor<UserObjectType>::validParams();
  params += LayeredBase::validParams();
  return params;
}

template <typename UserObjectType>
LayeredIntegralBase<UserObjectType>::LayeredIntegralBase(const InputParameters & parameters)
  : SpatialUserObjectFunctor<UserObjectType>(parameters), LayeredBase(parameters)
{
  if (parameters.isParamValid("block") && parameters.isParamValid("boundary"))
    mooseError("Both block and boundary cannot be specified for a layered integral user object. If "
               "you want to define the geometric bounds of the layers from a specified block set "
               "layer_bounding_block instead.");
}

template <typename UserObjectType>
void
LayeredIntegralBase<UserObjectType>::initialize()
{
  UserObjectType::initialize();
  LayeredBase::initialize();
}

template <typename UserObjectType>
void
LayeredIntegralBase<UserObjectType>::execute()
{
  const auto integral_value = computeIntegral();

  const auto layer = getLayer(_current_elem->vertex_average());

  setLayerValue(layer, getLayerValue(layer) + integral_value);
}

template <typename UserObjectType>
void
LayeredIntegralBase<UserObjectType>::finalize()
{
  LayeredBase::finalize();
}

template <typename UserObjectType>
void
LayeredIntegralBase<UserObjectType>::threadJoin(const UserObject & y)
{
  UserObjectType::threadJoin(y);
  LayeredBase::threadJoin(y);
}

template <typename UserObjectType>
const std::vector<Point>
LayeredIntegralBase<UserObjectType>::spatialPoints() const
{
  std::vector<Point> points;

  for (const auto & l : _layer_centers)
  {
    Point pt(0.0, 0.0, 0.0);
    pt(_direction) = l;
    points.push_back(pt);
  }

  return points;
}
