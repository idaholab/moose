//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"
#include "TagAuxBase.h"

/**
 * TagVectorAux returns the coupled DOF value of a tagged vector.
 */
class KokkosTagVectorAux : public TagAuxBase<Moose::Kokkos::AuxKernel>
{
public:
  static InputParameters validParams();

  KokkosTagVectorAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, ResidualDatum & datum) const;

protected:
  /// Whether to remove variable scaling from the returned value
  const bool _remove_variable_scaling;
  const Moose::Kokkos::VariableValue _v;
  const MooseVariableBase & _v_var;
  /// Variable scaling factor
  const Moose::Kokkos::Scalar<const Real> _scaling_factor;
};

KOKKOS_FUNCTION inline Real
KokkosTagVectorAux::computeValue(const unsigned int qp, ResidualDatum & datum) const
{
  return _remove_variable_scaling ? _v(datum, qp) / _scaling_factor : _v(datum, qp);
}
