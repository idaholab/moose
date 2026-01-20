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

template <typename T, typename index_type>
void
Array<T, 1, index_type, LayoutType::LEFT>::axby(const T a,
                                                const Array<T, 1, index_type, LayoutType::LEFT> & x,
                                                const char op,
                                                const T b,
                                                const Array<T, 1, index_type, LayoutType::LEFT> & y,
                                                const bool accumulate)
{
  if (!this->isDeviceAlloc() || !x.isDeviceAlloc() || !y.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for axby() should be on device.");

  if (!(this->size() == x.size() && (x.size() == y.size())))
    mooseError("Kokkos array error: all arrays for axby() should have the same size.");

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>> policy(0, this->size());

  auto data = this->deviceData();

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

template <typename T, typename index_type>
void
Array<T, 1, index_type, LayoutType::LEFT>::scal(const T a,
                                                const Array<T, 1, index_type, LayoutType::LEFT> & x)
{
  if (!this->isDeviceAlloc() || !x.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for scal() should be on device.");

  if (this->size() != x.size())
    mooseError("Kokkos array error: all arrays for scal() should have the same size.");

  ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(
      this->deviceData(), this->size());
  ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> other(x.deviceData(),
                                                                                   x.size());

  KokkosBlas::scal(self, a, other);
}

template <typename T, typename index_type>
void
Array<T, 1, index_type, LayoutType::LEFT>::scal(const T a)
{
  return scal(a, *this);
}

template <typename T, typename index_type>
T
Array<T, 1, index_type, LayoutType::LEFT>::dot(const Array<T, 1, index_type, LayoutType::LEFT> & x)
{
  if (!this->isDeviceAlloc() || !x.isDeviceAlloc())
    mooseError("Kokkos array error: all arrays for dot() should be on device.");

  if (this->size() != x.size())
    mooseError("Kokkos array error: all arrays for dot() should have the same size.");

  ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(
      this->deviceData(), this->size());
  ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> other(x.deviceData(),
                                                                                   x.size());

  return KokkosBlas::dot(self, other);
}

template <typename T, typename index_type>
T
Array<T, 1, index_type, LayoutType::LEFT>::nrm2()
{
  if (!this->isDeviceAlloc())
    mooseError("Kokkos array error: array for nrm2() should be on device.");

  ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> self(
      this->deviceData(), this->size());

  return KokkosBlas::nrm2(self);
}

template class Array<Real, 1, uint8_t, LayoutType::LEFT>;
template class Array<Real, 1, uint16_t, LayoutType::LEFT>;
template class Array<Real, 1, uint32_t, LayoutType::LEFT>;
template class Array<Real, 1, uint64_t, LayoutType::LEFT>;

} // namespace Kokkos
} // namespace Moose
