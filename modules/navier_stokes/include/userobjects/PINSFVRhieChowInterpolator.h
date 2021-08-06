//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVRhieChowInterpolator.h"
#include "CellCenteredMapFunctor.h"
#include <unordered_map>

/**
 * A class that inherits the free-flow class's implementation of Rhie-Chow data gathering and body
 * force interpolation and adds the ability to perform repeated interpolations and reconstructions
 * of the porosity in order to reduce non-physical oscillations that arise from property
 * discontinuities in a collocated discretization of pressure and velocity
 */
class PINSFVRhieChowInterpolator : public INSFVRhieChowInterpolator
{
public:
  static InputParameters validParams();
  PINSFVRhieChowInterpolator(const InputParameters & params);

  void meshChanged() override;
  void residualSetup() override;

protected:
  const Moose::FunctorBase<ADReal> & epsilon(THREAD_ID tid) const override;

  /// The thread 0 copy of the porosity functor held by the subproblem. Initially this functor
  /// should be provided by a functor material property or function. We then perform repeated
  /// interpolations and reconstructions and then reassign the resulting smoothed field to this
  /// functor and other thread's copies of this functor
  Moose::Functor<ADReal> & _eps;

  /// All the thread copies of the problem's porosity functor
  std::vector<const Moose::Functor<ADReal> *> _epss;

  /// The number of interpolations and reconstructions that should be performed on the porosity
  /// functor/field. One smoothing layer corresponds to one interpolation and one reconstruction
  const unsigned short _smoothing_layers;

  /// All the face information that are "geometrically" accessible on this process. For an internal
  /// face, we consider it to be geometrically accessible if neither element nor neighbor is a \p
  /// libMesh::remote_elem. For a boundary face, we consider the face to be geometrically accessible
  /// if the adjoining element and all its non-null neighbors are not \p libMesh::remote_elem. This
  /// is due to the need to be able to perform two-term expansions which require ability to compute
  /// a Green-Gauss gradient
  std::vector<const FaceInfo *> _geometric_fi;

  /// The smoothed porosity functor/field. After we construct this functor/field we assign it to the
  /// subproblem's porosity functor
  CellCenteredMapFunctor<ADReal, std::unordered_map<dof_id_type, ADReal>> _smoothed_eps;

private:
  /**
   * called during the first \p residualSetup and upon \p meshChanged, this method performs the
   * interpolations and reconstructions of porosity
   */
  void pinsfvSetup();
};

inline const Moose::FunctorBase<ADReal> &
PINSFVRhieChowInterpolator::epsilon(const THREAD_ID tid) const
{
  return *_epss[tid];
}
