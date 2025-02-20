//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredBase.h"
#include "InputParameters.h"
#include "MooseFunctor.h"

/**
 * Base class for computing layered side integrals
 */
template <typename UserObjectType>
class LayeredIntegralBase : public UserObjectType,
                            public LayeredBase,
                            public Moose::FunctorBase<Real>
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
  using ElemArg = Moose::ElemArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using ElemPointArg = Moose::ElemPointArg;
  using NodeArg = Moose::NodeArg;

  virtual Real evaluate(const ElemArg & elem, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const FaceArg & face, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemSideQpArg & elem_side_qp,
                        const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemPointArg & elem_point,
                        const Moose::StateArg & state) const override final;
  virtual Real evaluate(const NodeArg & node, const Moose::StateArg & state) const override final;

  virtual bool supportsFaceArg() const override final { return true; }
  virtual bool supportsElemSideQpArg() const override final { return true; }

private:
  /*
   * Helper template implementing functor evaluations
   */
  template <typename SpatialArg>
  Real evaluateTemplate(const SpatialArg & position, const Moose::StateArg & state) const;
};

template <typename UserObjectType>
InputParameters
LayeredIntegralBase<UserObjectType>::validParams()
{
  InputParameters params = UserObjectType::validParams();
  params += LayeredBase::validParams();
  return params;
}

template <typename UserObjectType>
LayeredIntegralBase<UserObjectType>::LayeredIntegralBase(const InputParameters & parameters)
  : UserObjectType(parameters), LayeredBase(parameters), Moose::FunctorBase<Real>(this->name())
{
  if (parameters.isParamValid("block") && parameters.isParamValid("boundary"))
    mooseError("Both block and boundary cannot be specified in LayeredSideIntegral. If you want to "
               "define the geometric bounds of the layers from a specified block set "
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

template <typename UserObjectType>
template <typename SpatialArg>
Real
LayeredIntegralBase<UserObjectType>::evaluateTemplate(const SpatialArg & position,
                                                      const Moose::StateArg & /*state*/) const
{
  return spatialValue(position.getPoint());
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const ElemArg & elem,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(elem, state);
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const FaceArg & face,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(face, state);
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const ElemQpArg & qp,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(qp, state);
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(elem_side_qp, state);
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const ElemPointArg & elem_point,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(elem_point, state);
}

template <typename UserObjectType>
Real
LayeredIntegralBase<UserObjectType>::evaluate(const NodeArg & node,
                                              const Moose::StateArg & state) const
{
  return evaluateTemplate(node, state);
}
