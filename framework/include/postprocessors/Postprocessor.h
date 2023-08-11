//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OutputInterface.h"
#include "FEProblemBase.h"
#include "NonADFunctorInterface.h"
#include "libmesh/parallel.h"

/**
 * Base class for all Postprocessors.  Defines a name and sets up the
 * virtual getValue() interface which must be overridden by derived
 * classes.
 */
class Postprocessor : public OutputInterface,
                      public NonADFunctorInterface,
                      public Moose::FunctorBase<Real>
{
public:
  static InputParameters validParams();

  Postprocessor(const MooseObject * moose_object);

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   *
   * This method will be removed in favor of the const version.
   */
  virtual PostprocessorValue getValue();

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() const;

  /**
   * Gets the current value stored in the ReporterData.
   *
   * In most (but not all) cases, this should return the same as \c getValue().
   */
  PostprocessorValue getCurrentReporterValue() const;

  /**
   * Returns the name of the Postprocessor.
   */
  std::string PPName() const { return _pp_name; }

protected:
  /// MOOSE object
  const MooseObject & _pp_moose_object;

  /// Post-processor name
  const std::string _pp_name;

  /// FE problem
  FEProblemBase & _pp_fe_problem;

  /// Reporter data
  ReporterData & _reporter_data;

private:
  using ElemArg = Moose::ElemArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using ElemPointArg = Moose::ElemPointArg;

  ValueType evaluate(const ElemArg & elem, const Moose::StateArg & state) const override final;
  ValueType evaluate(const FaceArg & face, const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const Moose::StateArg & state) const override final;
  ValueType evaluate(const ElemPointArg & elem_point,
                     const Moose::StateArg & state) const override final;

  GradientType evaluateGradient(const ElemArg & elem,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const FaceArg & face,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemQpArg & qp,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                const Moose::StateArg & state) const override final;
  GradientType evaluateGradient(const ElemPointArg & elem_point,
                                const Moose::StateArg & state) const override final;

  DotType evaluateDot(const ElemArg & elem, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const FaceArg & face, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp,
                      const Moose::StateArg & state) const override final;
  DotType evaluateDot(const ElemPointArg & elem_point,
                      const Moose::StateArg & state) const override final;

  /**
   * Gives a one-time warning for calling an \c evaluateDot() method.
   */
  void evaluateDotWarning() const
  {
    mooseDoOnce(
        mooseWarning("The time derivative functor operator was called on the post-processor '",
                     _pp_name,
                     "'. A zero value will always be returned, even if the post-processor value "
                     "changes with time."));
  }
};
