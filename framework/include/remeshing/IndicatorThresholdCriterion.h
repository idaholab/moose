//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RemeshCriterion.h"

template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;

/**
 * Fires when the field of an Adaptivity Indicator becomes large, or when an element becomes small
 * compared to its target size.
 *
 * The refinement test measures the maximum over the active elements of the elemental field written
 * by the [Adaptivity/Indicators] sub-block named by the 'indicator' parameter, which is reduced
 * over the communicator before it is compared to 'refine_threshold'.
 *
 * The over-refinement test is optional and is on only when both 'sizing_variable' and
 * 'coarsen_fraction' are supplied. It fires when the diameter of an active element falls below
 * 'coarsen_fraction' times the value of the target size field on that element. That test compares
 * every element to its own target size, so the flag itself is reduced over the communicator rather
 * than a measured quantity.
 *
 * The criterion only decides when the engine remeshes. The elements to refine and to coarsen are
 * selected by the remesher, which reads the same 'sizing_variable' field.
 */
class IndicatorThresholdCriterion : public RemeshCriterion
{
public:
  static InputParameters validParams();

  IndicatorThresholdCriterion(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual bool shouldRemesh() override;

  virtual bool consumesIndicators() const override { return true; }

private:
  /**
   * The variable named \p name, on the system built over evaluationMesh().
   *
   * The variable is looked up on every evaluation rather than cached, for the reason
   * evaluationMesh() is: the mesh the criterion runs on is the displaced mesh when the problem has
   * displacements, and the degree of freedom indices of the elements being looped over are the ones
   * of the system built over that mesh.
   */
  const MooseVariable & evaluationVariable(const std::string & name) const;

  /**
   * Error out when the field named by \p parameter is not a CONSTANT MONOMIAL variable, because it
   * is read as a single degree of freedom per element.
   *
   * @param parameter the parameter that named the field, for the error message
   * @param name the name of the field
   */
  void checkElementalField(const std::string & parameter, const std::string & name) const;

  /// The Indicator whose elemental field the refinement test measures
  const IndicatorName _indicator;

  /// The indicator value no active element is allowed to exceed
  const Real _refine_threshold;

  /// Whether 'sizing_variable' and 'coarsen_fraction' turn the over-refinement test on
  const bool _check_over_refinement;

  /// The target element size field, read only when _check_over_refinement
  const VariableName _sizing_variable;

  /// The fraction of its target size an element diameter is not allowed to fall below
  const Real _coarsen_fraction;
};
