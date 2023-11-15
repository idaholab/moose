//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshDivision.h"
#include "NonADFunctorInterface.h"

/**
 * Divides the mesh based on the binned values of a functor
 */
class FunctorBinnedValuesDivision : public MeshDivision, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  FunctorBinnedValuesDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /**
   * @brief Get the bin for that functor value
   * @param value functor value to bin
   * @param pt point where the functor is being evaluated, useful for warnings
   */
  unsigned int getBinIndex(Real value, const Point & pt) const;

  /// Min functor bin value
  const Real _min;
  /// Max functor bin value
  const Real _max;
  /// Number of value bins
  const unsigned int _nbins;
  /// Functor to use to subdivide the mesh
  const Moose::Functor<Real> & _functor;
  /// Whether to map functor values outside [min, max] onto the edge bins
  const bool _oob_is_edge_bins;
};
