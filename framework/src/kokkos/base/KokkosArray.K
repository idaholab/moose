//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBlas.hpp"

#include "KokkosArray.h"
#include "KokkosThread.h"

namespace Moose
{
namespace Kokkos
{

void
free(void * ptr)
{
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(ptr);
}

template <>
template <>
void
Array<Real, 1, LayoutType::LEFT>::axby<Real>(const Real a,
                                             const Array<Real, 1, LayoutType::LEFT> & x,
                                             const char op,
                                             const Real b,
                                             const Array<Real, 1, LayoutType::LEFT> & y,
                                             const bool accumulate)
{
  if (!isDeviceAlloc() || !x.isDeviceAlloc() || !y.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for axby() should be on device.");

  if (!(size() == x.size() && (x.size() == y.size())))
    mooseError("Kokkos array error: all arrays for axby() should have the same size.");

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>> policy(0, size());

  auto data = deviceData();

  switch (op)
  {
    case '+':
      ::Kokkos::parallel_for(
          policy, KOKKOS_LAMBDA(const ThreadID i) {
            if (accumulate)
              data[i] += (a * x[i]) + (b * y[i]);
            else
              data[i] = (a * x[i]) + (b * y[i]);
          });
      break;
    case '-':
      ::Kokkos::parallel_for(
          policy, KOKKOS_LAMBDA(const ThreadID i) {
            if (accumulate)
              data[i] += (a * x[i]) - (b * y[i]);
            else
              data[i] = (a * x[i]) - (b * y[i]);
          });
      break;
    case '*':
      ::Kokkos::parallel_for(
          policy, KOKKOS_LAMBDA(const ThreadID i) {
            if (accumulate)
              data[i] += (a * x[i]) * (b * y[i]);
            else
              data[i] = (a * x[i]) * (b * y[i]);
          });
      break;
    case '/':
      ::Kokkos::parallel_for(
          policy, KOKKOS_LAMBDA(const ThreadID i) {
            if (accumulate)
              data[i] += (a * x[i]) / (b * y[i]);
            else
              data[i] = (a * x[i]) / (b * y[i]);
          });
      break;
    default:
      mooseError("Kokkos array error: unknown operation '", op, "' for axby().");
  }

  ::Kokkos::fence();
}

template <>
template <>
void
Array<Real, 1, LayoutType::LEFT>::scal<Real>(const Real a,
                                             const Array<Real, 1, LayoutType::LEFT> & x)
{
  if (!isDeviceAlloc() || !x.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for scal() should be on device.");

  if (size() != x.size())
    mooseError("Kokkos array error: all arrays for scal() should have the same size.");

  ::Kokkos::View<Real *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(deviceData(),
                                                                                     size());
  ::Kokkos::View<Real *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> other(
      x.deviceData(), x.size());

  KokkosBlas::scal(self, a, other);
}

template <>
template <>
void
Array<Real, 1, LayoutType::LEFT>::scal<Real>(const Real a)
{
  return scal(a, *this);
}

template <>
template <>
Real
Array<Real, 1, LayoutType::LEFT>::dot<Real>(const Array<Real, 1, LayoutType::LEFT> & x)
{
  if (!isDeviceAlloc() || !x.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for dot() should be on device.");

  if (size() != x.size())
    mooseError("Kokkos array error: all arrays for dot() should have the same size.");

  ::Kokkos::View<Real *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(deviceData(),
                                                                                     size());
  ::Kokkos::View<Real *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> other(
      x.deviceData(), x.size());

  return KokkosBlas::dot(self, other);
}

template <>
template <>
Real
Array<Real, 1>::nrm2<Real>()
{
  if (!isDeviceAlloc())
    mooseError("Kokkos array error: array for nrm2() should be on device.");

  ::Kokkos::View<Real *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(deviceData(),
                                                                                     size());

  return KokkosBlas::nrm2(self);
}

template class Array<Real, 1, LayoutType::LEFT>;

} // namespace Kokkos
} // namespace Moose
