//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementVectorPostprocessor.h"
#include "KokkosMaterialPropertyValue.h"

#include "MooseMesh.h"

class KokkosExtraIDIntegralVectorPostprocessor : public Moose::Kokkos::ElementVectorPostprocessor
{
  template <typename T>
  using Array = Moose::Kokkos::Array<T>;

  template <typename T>
  using MaterialProperty = Moose::Kokkos::MaterialProperty<T>;

  using VariableValue = Moose::Kokkos::VariableValue;

public:
  static InputParameters validParams();
  KokkosExtraIDIntegralVectorPostprocessor(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void compute() override;
  virtual void finalize() override;

  KOKKOS_FUNCTION void join(ReducerLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(ReducerLoop, Real * result) const;
  KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const;
  KOKKOS_FUNCTION void execute(Datum & datum) const;

protected:
  /// MOOSE mesh
  const MooseMesh & _mesh;
  /// Whether or not to compute volume average
  const bool _average;
  /// Calculation mode
  const MooseEnum _mode;
  /// Number of variables to be integrated
  const unsigned int _nvar;
  /// Number of material properties to be integrated
  const unsigned int _nprop;
  /// Name of material properties
  const std::vector<MaterialPropertyName> _prop_names;
  /// Extra IDs in use
  const std::vector<ExtraElementIDName> _extra_id;
  /// Number of extra IDs in use
  const unsigned int _n_extra_id;
  /// Map of element ids to parsed vpp ids
  std::unordered_map<dof_id_type, dof_id_type> _unique_vpp_id_map;
  Array<Array<dof_id_type>> _unique_vpp_ids;
  /// Quadrature point values of coupled MOOSE variables
  VariableValue _var_values;
  /// Material properties to be integrated
  Array<MaterialProperty<Real>> _props;
  /// Vector holding the volume of extra IDs
  Array<Real> _volumes;
  /// Vectors holding integrals over extra IDs
  Array<Array<Real>> _integrals;
  /// Vectors holding extra IDs
  std::vector<VectorPostprocessorValue *> _extra_ids;
  /// Size of each vector
  dof_id_type _vector_size;
  /// Sum cache size
  static constexpr unsigned int _cache_size = 10;
};

KOKKOS_FUNCTION inline void
KokkosExtraIDIntegralVectorPostprocessor::join(ReducerLoop,
                                               Real * result,
                                               const Real * source) const
{
  auto size = _vector_size * (_nvar + _nprop + _average);

  for (decltype(size) i = 0; i < size; ++i)
    result[i] += source[i];
}

KOKKOS_FUNCTION inline void
KokkosExtraIDIntegralVectorPostprocessor::init(ReducerLoop, Real * result) const
{
  auto size = _vector_size * (_nvar + _nprop + _average);

  for (decltype(size) i = 0; i < size; ++i)
    result[i] = 0;
}

KOKKOS_FUNCTION inline void
KokkosExtraIDIntegralVectorPostprocessor::reduce(Datum & datum, Real * result) const
{
  const auto ipos = _unique_vpp_ids[datum.subdomain()](datum.elemID());
  const auto & var = _var_values.variable();
  const auto subdomain = datum.subdomain();

  Real sum[_cache_size] = {0};
  Real vol = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit();

    const auto JxW = datum.JxW(qp);

    unsigned int i = 0;

    for (unsigned int ivar = 0; ivar < _nvar; ++ivar, ++i)
      if (kokkosSystem(var.sys(ivar)).isVariableActive(var.var(ivar), subdomain))
        sum[i] += JxW * _var_values(datum, qp, ivar);

    for (unsigned int iprop = 0; iprop < _nprop; ++iprop, ++i)
      sum[i] += JxW * _props[iprop](datum, qp);

    vol += JxW;
  }

  for (unsigned int i = 0; i < _nvar + _nprop; ++i)
    result[ipos + i * _vector_size] += sum[i];

  if (_average)
    result[ipos + (_nvar + _nprop) * _vector_size] += vol;
}

KOKKOS_FUNCTION inline void
KokkosExtraIDIntegralVectorPostprocessor::execute(Datum & datum) const
{
  const auto ipos = _unique_vpp_ids[datum.subdomain()](datum.elemID());
  const auto & var = _var_values.variable();
  const auto subdomain = datum.subdomain();

  Real sum[_cache_size] = {0};
  Real vol = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit();

    const auto JxW = datum.JxW(qp);

    unsigned int i = 0;

    for (unsigned int ivar = 0; ivar < _nvar; ++ivar, ++i)
      if (kokkosSystem(var.sys(ivar)).isVariableActive(var.var(ivar), subdomain))
        sum[i] += JxW * _var_values(datum, qp, ivar);

    for (unsigned int iprop = 0; iprop < _nprop; ++iprop, ++i)
      sum[i] += JxW * _props[iprop](datum, qp);

    vol += JxW;
  }

  for (unsigned int i = 0; i < _nvar + _nprop; ++i)
    Kokkos::atomic_add(&_integrals[i][ipos], sum[i]);

  if (_average)
    Kokkos::atomic_add(&_volumes[ipos], vol);
}
