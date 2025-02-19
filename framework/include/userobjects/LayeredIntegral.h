//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "LayeredBase.h"
#include "MooseFunctor.h"

/**
 * This UserObject computes volume integrals of a variable storing partial sums for the specified
 * number of intervals in a direction (x,y,z).
 */
class LayeredIntegral : public ElementIntegralVariableUserObject,
                        public LayeredBase,
                        public Moose::FunctorBase<Real>
{
public:
  static InputParameters validParams();

  LayeredIntegral(const InputParameters & parameters);

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

  virtual bool hasBlocks(SubdomainID sub) const override;

protected:
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
