//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseFunctor.h"
#include "MooseTypes.h"

class SinglePhaseFluidProperties;

/**
 * Wall heat transfer coefficient from the Dittus-Boelter correlation for a lumped,
 * path-integrated flow segment.
 *
 * The segment is zero-dimensional: every input is either a coupled scalar variable or a
 * functor of the segment's geometry, so the coefficient has no spatial dependence. Each
 * evaluate() overload therefore discards its spatial argument. That is what makes this
 * functor usable from a ScalarKernel, which has no current element and so can only supply
 * a null-element argument; a functor material property would instead be a
 * PiecewiseByBlockLambdaFunctor and would dereference that null element to select its
 * per-block lambda.
 */
class DittusBoelterHTCFunctor : public Moose::FunctorBase<ADReal>
{
public:
  /**
   * @param name Name to register this functor under
   * @param fp Fluid properties of the flowing fluid
   * @param m Mass flow rate through the segment [kg/s]
   * @param T_fluid Fluid temperature of the segment [K]
   * @param T_fluid_inlet Fluid temperature at the nominal inlet neighbor [K]
   * @param T_fluid_outlet Fluid temperature at the nominal outlet neighbor [K]
   * @param P_ref System reference pressure [Pa]
   * @param flow_area Cross-sectional area of the flow channel [m^2]
   * @param wetted_perimeter Wetted perimeter of the flow channel [m]
   */
  DittusBoelterHTCFunctor(const MooseFunctorName & name,
                          const SinglePhaseFluidProperties & fp,
                          const ADVariableValue & m,
                          const ADVariableValue & T_fluid,
                          const ADVariableValue & T_fluid_inlet,
                          const ADVariableValue & T_fluid_outlet,
                          const Moose::Functor<ADReal> & P_ref,
                          const Moose::Functor<ADReal> & flow_area,
                          const Moose::Functor<ADReal> & wetted_perimeter);

  bool hasBlocks(SubdomainID) const override { return true; }

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return true; }

private:
  ValueType evaluate(const Moose::ElemArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }
  ValueType evaluate(const Moose::FaceArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }
  ValueType evaluate(const Moose::ElemQpArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }
  ValueType evaluate(const Moose::ElemSideQpArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }
  ValueType evaluate(const Moose::ElemPointArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }
  ValueType evaluate(const Moose::NodeArg &, const Moose::StateArg & state) const override
  {
    return evaluate(state);
  }

  /// Evaluates the correlation; the sole implementation behind every overload above
  ValueType evaluate(const Moose::StateArg & state) const;

  /// Fluid properties of the flowing fluid
  const SinglePhaseFluidProperties & _fp;
  /// Mass flow rate through the segment
  const ADVariableValue & _m;
  /// Fluid temperature of the segment
  const ADVariableValue & _T_fluid;
  /// Fluid temperature at the nominal inlet neighbor
  const ADVariableValue & _T_fluid_inlet;
  /// Fluid temperature at the nominal outlet neighbor
  const ADVariableValue & _T_fluid_outlet;
  /// System reference pressure
  const Moose::Functor<ADReal> & _P_ref;
  /// Cross-sectional area of the flow channel
  const Moose::Functor<ADReal> & _flow_area;
  /// Wetted perimeter of the flow channel
  const Moose::Functor<ADReal> & _wetted_perimeter;
};
