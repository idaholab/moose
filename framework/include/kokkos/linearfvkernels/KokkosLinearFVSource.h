//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVKernel.h"
#include "KokkosFunction.h"

class KokkosLinearFVSource : public Moose::Kokkos::LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  KokkosLinearFVSource(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum &) const { return 0; }

  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum & datum) const
  {
    const auto elem = datum.elemID();
    const auto centroid = datum.mesh().getElementCentroid(elem);
    return _scale.value(_t, centroid) * _source_density.value(_t, centroid) *
           datum.mesh().getElementVolume(elem);
  }

private:
  const Moose::Kokkos::Function _source_density;
  const Moose::Kokkos::Function _scale;
};
