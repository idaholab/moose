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
#include "NonADFunctorInterface.h"
#include "libmesh/parallel.h"

class MooseObject;

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
   * This will get called to actually grab the final value the postprocessor has calculated
   *
   * Note that this should only be called by internal methods, namely the problem that
   * actually sets the value globally for other things to use. If you want the value
   * outside of one of these external methods, you should use getCurrentValue().
   *
   * This method will be removed in favor of the const version.
   */
  virtual PostprocessorValue getValue();

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   *
   * Note that this should only be called by internal methods, namely the problem that
   * actually sets the value globally for other things to use. If you want the value
   * outside of one of these external methods, you should use getCurrentValue().
   */
  virtual PostprocessorValue getValue() const;

  /**
   * @return The "current" value of this Postprocessor.
   *
   * Your sanity would tell you... why not just call getValue()? Well - the intention
   * of getValue() is to be called by the problem when the UserObjects are executed,
   * and not by other things. This enables the control of _when_ this Postprocessor
   * is updated, which could be very important. If the implementation of getValue() is
   * such that it actually computes a new value (instead of one that is called in
   * finalize()), you could potentially call getValue() and not get the value as it
   * was at the last time this PP was executed.
   *
   * What this does instead is gives you the value that was last set as this PP was
   * executed by the problem. That is, the value that every object that uses the
   * PostprocessorInterface will get you.
   */
  const PostprocessorValue & getCurrentValue() const { return _current_value; }

  /**
   * Returns the name of the Postprocessor.
   */
  const std::string & PPName() const { return _pp_name; }

  virtual bool hasBlocks(SubdomainID /* id */) const override { return true; }

protected:
  /// Post-processor name
  const std::string & _pp_name;

  /// The current value, which is the Reporter value that changes when we execute UOs in the problem
  const PostprocessorValue & _current_value;

private:
  /**
   * Internal method to be used to declare the value and store it within _current_value in the
   * constructor.
   */
  const PostprocessorValue & declareValue(const MooseObject & moose_object);

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
   * Internal method for giving a one-time warning for calling an \c evaluateDot() method.
   */
  void evaluateDotWarning() const;

  /// MOOSE object
  const MooseObject & _pp_moose_object;
};
