//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_SCOPE
#include "GPUHeader.h"
#endif

#include "Conversion.h"
#include "DataIO.h"

#include <cstdint>

#define usingKokkosArrayBaseMembers(T, dimension)                                                  \
private:                                                                                           \
  using ArrayBase<T, dimension>::_n;                                                               \
  using ArrayBase<T, dimension>::_s;                                                               \
  using ArrayBase<T, dimension>::_d;                                                               \
                                                                                                   \
public:                                                                                            \
  using ArrayBase<T, dimension>::create;                                                           \
  using ArrayBase<T, dimension>::createHost;                                                       \
  using ArrayBase<T, dimension>::createDevice;                                                     \
  using ArrayBase<T, dimension>::offset;                                                           \
  using ArrayBase<T, dimension>::operator=;

namespace Moose
{
namespace Kokkos
{

void free(void * ptr);

enum class MemcpyKind
{
  HOST_TO_DEVICE,
  DEVICE_TO_HOST,
  DEVICE_TO_DEVICE
};

template <typename T, unsigned int dimension = 1>
class Array
{
};

template <typename T>
struct ArrayDeepCopy
{
  static const bool value = false;
};

template <typename T, unsigned int dimension>
struct ArrayDeepCopy<Array<T, dimension>>
{
  static const bool value = ArrayDeepCopy<T>::value;
};

template <typename T, unsigned int dimension>
class ArrayBase
{
public:
  // Iterator for host data
  class iterator
  {
  private:
    T * it;

  public:
    iterator(T * it) : it(it) {}
    bool operator==(const iterator & other) const { return it == other.it; }
    bool operator!=(const iterator & other) const { return it != other.it; }
    T & operator*() const { return *it; }
    iterator & operator++()
    {
      ++it;
      return *this;
    }
    iterator operator++(int)
    {
      iterator pre = *this;
      ++it;
      return pre;
    }
  };
  iterator begin() const { return iterator(_host_data); }
  iterator end() const { return iterator(_host_data + _size); }

private:
  // Reference counter
  unsigned int * _counter = nullptr;
  // Flag whether host data was allocated
  bool _is_host_alloc = false;
  // Flag whether device data was allocated
  bool _is_device_alloc = false;
  // Flag whether host data pointer is an alias of another pointer
  bool _is_host_alias = false;
  // Flag whether device data pointer is an alias of another pointer
  bool _is_device_alias = false;
  // Host data pointer
  T * _host_data = nullptr;
  // Device data pointer
  T * _device_data = nullptr;
  // Total size
  uint64_t _size = 0;

protected:
  // Size of each dimension
  uint64_t _n[dimension] = {0};
  // Stride of each dimension
  uint64_t _s[dimension] = {0};
  // Offset of each dimension
  int64_t _d[dimension] = {0};

public:
  // Free all array data
  void destroy();
  // Destructor
  ~ArrayBase() { destroy(); }

#ifdef MOOSE_KOKKOS_SCOPE
protected:
  template <bool host, bool device>
  void createInternal(const std::vector<uint64_t> n);
  void createInternal(const std::vector<uint64_t> n, bool host, bool device);
  template <typename TargetSpace, typename SourceSpace>
  void copyInternal(T * target, const T * source, uint64_t n);

public:
  // Default constructor
  ArrayBase() {}
  // Copy constructor
  ArrayBase(const ArrayBase<T, dimension> & array)
  {
    if constexpr (ArrayDeepCopy<T>::value)
      deepCopy(array);
    else
      shallowCopy(array);
  }

public:
  KOKKOS_FUNCTION bool isAlloc() const { return _is_host_alloc || _is_device_alloc; }
  KOKKOS_FUNCTION bool isHostAlloc() const { return _is_host_alloc; }
  KOKKOS_FUNCTION bool isDeviceAlloc() const { return _is_device_alloc; }
  KOKKOS_FUNCTION bool isHostAlias() const { return _is_host_alias; }
  KOKKOS_FUNCTION bool isDeviceAlias() const { return _is_device_alias; }
  KOKKOS_FUNCTION uint64_t size() const { return _size; }
  KOKKOS_FUNCTION uint64_t n(unsigned int dim) const { return _n[dim]; }
  KOKKOS_FUNCTION T * data() const
  {
    KOKKOS_IF_ON_DEVICE(return _device_data;)
    KOKKOS_IF_ON_HOST(return _host_data;)
  }
  KOKKOS_FUNCTION T & first() const
  {
    KOKKOS_IF_ON_DEVICE(return _device_data[0];)
    KOKKOS_IF_ON_HOST(return _host_data[0];)
  }
  KOKKOS_FUNCTION T & last() const
  {
    KOKKOS_IF_ON_DEVICE(return _device_data[_size - 1];)
    KOKKOS_IF_ON_HOST(return _host_data[_size - 1];)
  }
  KOKKOS_FUNCTION T & operator[](size_t i) const
  {
    KOKKOS_ASSERT(i < _size);

    KOKKOS_IF_ON_DEVICE(return _device_data[i];)
    KOKKOS_IF_ON_HOST(return _host_data[i];)
  }

public:
  // Return reference counter
  unsigned int use_count() const { return _counter ? *_counter : 0; }
  // Get host data pointer
  T * host_data() const { return _host_data; }
  // Get device data pointer
  T * device_data() const { return _device_data; }
  // Create array on host and device
  void create(const std::vector<uint64_t> n) { createInternal<true, true>(n); }
  // Create array on host
  void createHost(const std::vector<uint64_t> n) { createInternal<true, false>(n); }
  // Create array on device
  void createDevice(const std::vector<uint64_t> n) { createInternal<false, true>(n); }
  // Simply alias host data pointer for an initialized array
  void aliasHost(T * ptr);
  // Simply alias device data pointer for an initialized array
  void aliasDevice(T * ptr);
  // Simply allocate host data storage for an initialized array
  void allocHost();
  // Simply allocate device data storage for an initialized array
  void allocDevice();
  // Apply offsets to each dimension
  void offset(const std::vector<int64_t> d);
  // Copy between host and device buffers
  void copy(MemcpyKind dir = MemcpyKind::HOST_TO_DEVICE);
  // Copy in from external buffer
  void copyIn(const T * ptr, MemcpyKind dir, uint64_t n, uint64_t offset = 0);
  // Copy out to external buffer
  void copyOut(T * ptr, MemcpyKind dir, uint64_t n, uint64_t offset = 0);
  // Copy of nested Kokkos arrays
  void copyNested();
  // Shallow copy a Kokkos arrays
  void shallowCopy(const ArrayBase<T, dimension> & array);
  // Deep copy a Kokkos Array
  void deepCopy(const ArrayBase<T, dimension> & array);
  // Swap two Arrays
  void swap(ArrayBase<T, dimension> & array);

public:
  // Scalar copy operator
  auto & operator=(const T & scalar);
  // Shallow copy operator
  auto & operator=(const ArrayBase<T, dimension> & array)
  {
    shallowCopy(array);

    return *this;
  }
#endif
};

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::destroy()
{
  if (!_counter)
    return;

  _size = 0;

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _n[i] = 0;
    _s[i] = 0;
    _d[i] = 0;
  }

  if (*_counter > 1)
  {
    _host_data = nullptr;
    _device_data = nullptr;

    (*_counter)--;
    _counter = nullptr;
  }
  else if (*_counter == 1)
  {
    if (_is_host_alloc && !_is_host_alias)
    {
      if constexpr (std::is_default_constructible<T>::value)
        // Allocated by new
        delete[] _host_data;
      else
      {
        // Allocated by malloc
        for (uint64_t i = 0; i < _size; ++i)
          _host_data[i].~T();

        std::free(_host_data);
      }
    }

#ifdef MOOSE_HAVE_KOKKOS
    if (_is_device_alloc && !_is_device_alias)
      Moose::Kokkos::free(_device_data);
#endif

    delete _counter;
    _counter = nullptr;
  }

  _is_host_alloc = false;
  _is_device_alloc = false;
  _is_host_alias = false;
  _is_device_alias = false;
}

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::aliasHost(T * ptr)
{
  if (_is_host_alloc && !_is_host_alias)
    mooseError("Kokkos array error: cannot alias host data because host data was not aliased.");

  _host_data = ptr;
  _is_host_alloc = true;
  _is_host_alias = true;
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::aliasDevice(T * ptr)
{
  if (_is_device_alloc && !_is_device_alias)
    mooseError("Kokkos array error: cannot alias device data because device data was not aliased.");

  _device_data = ptr;
  _is_device_alloc = true;
  _is_device_alias = true;
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::allocHost()
{
  if (_is_host_alloc)
    return;

  if constexpr (std::is_default_constructible<T>::value)
    _host_data = new T[_size];
  else
    _host_data = static_cast<T *>(std::malloc(_size * sizeof(T)));

  _is_host_alloc = true;
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::allocDevice()
{
  if (_is_device_alloc)
    return;

  _device_data = static_cast<T *>(::Kokkos::kokkos_malloc(_size * sizeof(T)));

  _is_device_alloc = true;
}

template <typename T, unsigned int dimension>
template <bool host, bool device>
void
ArrayBase<T, dimension>::createInternal(const std::vector<uint64_t> n)
{
  if (n.size() != dimension)
    mooseError("Kokkos array error: the number of dimensions provided (",
               n.size(),
               ") must match the array dimension (",
               dimension,
               ").");

  if (_counter)
    destroy();

  _counter = new unsigned int;
  *_counter = 1;

  _size = 1;
  _s[0] = 1;

  for (unsigned int i = 0; i < n.size(); ++i)
  {
    _n[i] = n[i];
    _size *= n[i];

    if (i)
      _s[i] = _s[i - 1] * _n[i - 1];
  }

  if constexpr (host)
    allocHost();

  if constexpr (device)
    allocDevice();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::createInternal(const std::vector<uint64_t> n, bool host, bool device)
{
  if (host && device)
    createInternal<true, true>(n);
  else if (host && !device)
    createInternal<true, false>(n);
  else if (!host && device)
    createInternal<false, true>(n);
  else
    createInternal<false, false>(n);
}

template <typename T, unsigned int dimension>
template <typename TargetSpace, typename SourceSpace>
void
ArrayBase<T, dimension>::copyInternal(T * target, const T * source, uint64_t n)
{
  ::Kokkos::Impl::DeepCopy<TargetSpace, SourceSpace>(target, source, n * sizeof(T));
  ::Kokkos::fence();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::offset(const std::vector<int64_t> d)
{
  if (d.size() > dimension)
    mooseError("Kokkos array error: the number of offsets provided (",
               d.size(),
               ") cannot be larger than the array dimension (",
               dimension,
               ").");

  for (unsigned int i = 0; i < d.size(); ++i)
    _d[i] = d[i];
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copy(MemcpyKind dir)
{
  if (dir == MemcpyKind::HOST_TO_DEVICE)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // If device side memory is not allocated,
    if (!_is_device_alloc)
    {
      if (*_counter == 1)
        // allocate memory if this array is not shared with other arrays
        allocDevice();
      else
        // print error if this array is shared with other arrays
        mooseError("Kokkos array error: cannot copy from host to device because device side memory "
                   "was not allocated and array is being shared with other arrays.");
    }

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(_device_data, _host_data, _size);
  }
  else if (dir == MemcpyKind::DEVICE_TO_HOST)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // If host side memory is not allocated,
    if (!_is_host_alloc)
    {
      if (*_counter == 1)
        // allocate memory if this array is not shared with other arrays
        allocHost();
      else
        // print error if this array is shared with other arrays
        mooseError("Kokkos array error: cannot copy from device to host because host side memory "
                   "was not allocated and array is being shared with other arrays.");
    }

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(_host_data, _device_data, _size);
  }
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyIn(const T * ptr, MemcpyKind dir, uint64_t n, uint64_t offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copyin data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyKind::HOST_TO_DEVICE)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(_device_data + offset, ptr, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_DEVICE)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from device to device
    copyInternal<MemSpace, MemSpace>(_device_data + offset, ptr, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_HOST)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(_host_data + offset, ptr, n);
  }
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyOut(T * ptr, MemcpyKind dir, uint64_t n, uint64_t offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copyout data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyKind::DEVICE_TO_HOST)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(ptr, _device_data + offset, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_DEVICE)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from device to device
    copyInternal<MemSpace, MemSpace>(ptr, _device_data + offset, n);
  }
  else if (dir == MemcpyKind::HOST_TO_DEVICE)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(ptr, _host_data + offset, n);
  }
}

template <typename T>
void
copyInner(T & data)
{
}

template <typename T, unsigned int dimension>
void
copyInner(Array<T, dimension> & data)
{
  data.copyNested();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyNested()
{
  for (unsigned int i = 0; i < _size; ++i)
    copyInner(_host_data[i]);

  copy();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::shallowCopy(const ArrayBase<T, dimension> & array)
{
  destroy();

  _counter = array._counter;

  if (_counter)
    (*_counter)++;

  _size = array._size;

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _n[i] = array._n[i];
    _s[i] = array._s[i];
    _d[i] = array._d[i];
  }

  _is_host_alloc = array._is_host_alloc;
  _is_device_alloc = array._is_device_alloc;
  _is_host_alias = array._is_host_alias;
  _is_device_alias = array._is_device_alias;

  _host_data = array._host_data;
  _device_data = array._device_data;
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::deepCopy(const ArrayBase<T, dimension> & array)
{
  if (ArrayDeepCopy<T>::value && !array._is_host_alloc)
    mooseError(
        "Kokkos array error: cannot deep copy using constructor from array without host data.");

  std::vector<uint64_t> n(std::begin(array._n), std::end(array._n));

  createInternal(n, array._is_host_alloc, array._is_device_alloc);

  if constexpr (ArrayDeepCopy<T>::value)
  {
    for (uint64_t i = 0; i < _size; ++i)
      new (_host_data + i) T(array._host_data[i]);

    copy();
  }
  else
  {
    if (_is_host_alloc)
      std::memcpy(_host_data, array._host_data, _size * sizeof(T));

    if (_is_device_alloc)
      copyInternal<MemSpace, MemSpace>(_device_data, array._device_data, _size);
  }

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _d[i] = array._d[i];
    _s[i] = array._s[i];
  }
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::swap(ArrayBase<T, dimension> & array)
{
  ArrayBase<T, dimension> clone;

  clone.shallowCopy(*this);
  this->shallowCopy(array);
  array.shallowCopy(clone);
}

template <typename T, unsigned int dimension>
auto &
ArrayBase<T, dimension>::operator=(const T & scalar)
{
  if (_is_host_alloc)
    std::fill_n(_host_data, _size, scalar);

  if (_is_device_alloc)
  {
    ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>> data(_device_data,
                                                                                    _size);
    ::Kokkos::Experimental::fill_n(::Kokkos::DefaultExecutionSpace(), data, _size, scalar);
  }

  return *this;
}

using ::dataStore;

template <typename T, unsigned int dimension>
void
dataStore(std::ostream & stream, Array<T, dimension> & array, void * context)
{
  bool is_alloc = array.isAlloc();
  dataStore(stream, is_alloc, nullptr);

  if (!is_alloc)
    return;

  std::string type = typeid(T).name();
  dataStore(stream, type, nullptr);

  unsigned int dim = dimension;
  dataStore(stream, dim, nullptr);

  for (unsigned int dim = 0; dim < dimension; ++dim)
  {
    auto n = array.n(dim);
    dataStore(stream, n, nullptr);
  }

  if (array.isDeviceAlloc())
  {
    // We use malloc/free because we just want a memory copy
    // If T is a Kokkos array and we use new/delete or vector to copy it out,
    // the arrays will be destroyed on cleanup

    T * data = static_cast<T *>(std::malloc(array.size() * sizeof(T)));

    array.copyOut(data, MemcpyKind::DEVICE_TO_HOST, array.size());

    for (uint64_t i = 0; i < array.size(); ++i)
      dataStore(stream, data[i], context);

    std::free(data);
  }
  else
  {
    for (auto & value : array)
      dataStore(stream, value, context);
  }
}

using ::dataLoad;

template <typename T, unsigned int dimension>
void
dataLoad(std::istream & stream, Array<T, dimension> & array, void * context)
{
  bool is_alloc;
  dataLoad(stream, is_alloc, nullptr);

  if (!is_alloc)
    return;

  std::string from_type_name;
  dataLoad(stream, from_type_name, nullptr);

  if (from_type_name != typeid(T).name())
    mooseError("Kokkos array error: cannot load an array because the stored array is of type '",
               MooseUtils::prettyCppType(libMesh::demangle(from_type_name.c_str())),
               "' but the loading array is of type '",
               MooseUtils::prettyCppType(libMesh::demangle(typeid(T).name())),
               "'.");

  unsigned int from_dimension;
  dataLoad(stream, from_dimension, nullptr);

  if (from_dimension != dimension)
    mooseError("Kokkos array error: cannot load an array because the stored array is ",
               from_dimension,
               "D but the loading array is ",
               dimension,
               "D.");

  std::vector<uint64_t> from_n(dimension);
  std::vector<uint64_t> n(dimension);

  for (unsigned int dim = 0; dim < dimension; ++dim)
  {
    dataLoad(stream, from_n[dim], nullptr);
    n[dim] = array.n(dim);
  }

  if (from_n != n)
    mooseError("Kokkos array error: cannot load an array because the stored array has dimensions (",
               Moose::stringify(from_n),
               ") but the loading array has dimensions (",
               Moose::stringify(n),
               ").");

  if (array.isHostAlloc())
  {
    for (auto & value : array)
      dataLoad(stream, value, context);

    if (array.isDeviceAlloc())
      array.copy();
  }
  else
  {
    std::vector<T> data(array.size());

    for (auto & value : data)
      dataLoad(stream, value, context);

    array.copyIn(data.data(), MemcpyKind::HOST_TO_DEVICE, array.size());
  }
}
#endif

template <typename T>
class Array<T, 1> : public ArrayBase<T, 1>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 1);

public:
  Array(uint64_t n0) { create(n0); }
  Array(const std::vector<T> & vector) { *this = vector; }
  Array() {}

  void init(uint64_t n0) { this->template createInternal<false, false>({n0}); }
  void create(uint64_t n0) { this->template createInternal<true, true>({n0}); }
  void createHost(uint64_t n0) { this->template createInternal<true, false>({n0}); }
  void createDevice(uint64_t n0) { this->template createInternal<false, true>({n0}); }
  void offset(int64_t d0) { ArrayBase<T, 1>::offset({d0}); }

  template <bool host = true, bool device = true>
  void copyVector(const std::vector<T> & vector)
  {
    this->template createInternal<host, device>({vector.size()});

    if (host)
      std::memcpy(this->host_data(), vector.data(), this->size() * sizeof(T));

    if (device)
      this->template copyInternal<MemSpace, ::Kokkos::HostSpace>(
          this->device_data(), vector.data(), this->size());
  }
  template <bool host = true, bool device = true>
  void copySet(const std::set<T> & set)
  {
    std::vector<T> vector(set.begin(), set.end());

    copyVector<host, device>(vector);
  }

  auto & operator=(const std::vector<T> & vector)
  {
    copyVector(vector);

    return *this;
  }
  auto & operator=(const std::set<T> & set)
  {
    copySet(set);

    return *this;
  }

  KOKKOS_FUNCTION T & operator()(int64_t i0) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && i0 - _d[0] < _n[0]);

    return this->operator[](i0 - _d[0]);
  }
#endif
};

template <typename T>
class Array<T, 2> : public ArrayBase<T, 2>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 2);

public:
  Array(uint64_t n0, uint64_t n1) { create(n0, n1); }
  Array() {}

  void init(uint64_t n0, uint64_t n1) { this->template createInternal<false, false>({n0, n1}); }
  void create(uint64_t n0, uint64_t n1) { this->template createInternal<true, true>({n0, n1}); }
  void createHost(uint64_t n0, uint64_t n1)
  {
    this->template createInternal<true, false>({n0, n1});
  }
  void createDevice(uint64_t n0, uint64_t n1)
  {
    this->template createInternal<false, true>({n0, n1});
  }
  void offset(int64_t d0, int64_t d1) { ArrayBase<T, 2>::offset({d0, d1}); }

  KOKKOS_FUNCTION T & operator()(int64_t i0, int64_t i1) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && i0 - _d[0] < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && i1 - _d[1] < _n[1]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1]);
  }
#endif
};

template <typename T>
class Array<T, 3> : public ArrayBase<T, 3>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 3);

public:
  Array(uint64_t n0, uint64_t n1, uint64_t n2) { create(n0, n1, n2); }
  Array() {}

  void init(uint64_t n0, uint64_t n1, uint64_t n2)
  {
    this->template createInternal<false, false>({n0, n1, n2});
  }
  void create(uint64_t n0, uint64_t n1, uint64_t n2)
  {
    this->template createInternal<true, true>({n0, n1, n2});
  }
  void createHost(uint64_t n0, uint64_t n1, uint64_t n2)
  {
    this->template createInternal<true, false>({n0, n1, n2});
  }
  void createDevice(uint64_t n0, uint64_t n1, uint64_t n2)
  {
    this->template createInternal<false, true>({n0, n1, n2});
  }
  void offset(int64_t d0, int64_t d1, int64_t d2) { ArrayBase<T, 3>::offset({d0, d1, d2}); }

  KOKKOS_FUNCTION T & operator()(int64_t i0, int64_t i1, int64_t i2) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && i0 - _d[0] < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && i1 - _d[1] < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && i2 - _d[2] < _n[2]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1] + (i2 - _d[2]) * _s[2]);
  }
#endif
};

template <typename T>
class Array<T, 4> : public ArrayBase<T, 4>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 4);

public:
  Array(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3) { create(n0, n1, n2, n3); }
  Array() {}

  void init(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3)
  {
    this->template createInternal<false, false>({n0, n1, n2, n3});
  }
  void create(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3)
  {
    this->template createInternal<true, true>({n0, n1, n2, n3});
  }
  void createHost(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3)
  {
    this->template createInternal<true, false>({n0, n1, n2, n3});
  }
  void createDevice(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3)
  {
    this->template createInternal<false, true>({n0, n1, n2, n3});
  }
  void offset(int64_t d0, int64_t d1, int64_t d2, int64_t d3)
  {
    ArrayBase<T, 4>::offset({d0, d1, d2, d3});
  }

  KOKKOS_FUNCTION T & operator()(int64_t i0, int64_t i1, int64_t i2, int64_t i3) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && i0 - _d[0] < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && i1 - _d[1] < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && i2 - _d[2] < _n[2]);
    KOKKOS_ASSERT(i3 - _d[3] >= 0 && i3 - _d[3] < _n[3]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1] + (i2 - _d[2]) * _s[2] +
                            (i3 - _d[3]) * _s[3]);
  }
#endif
};

template <typename T>
class Array<T, 5> : public ArrayBase<T, 5>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 5);

public:
  Array(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4)
  {
    create(n0, n1, n2, n3, n4);
  }
  Array() {}

  void init(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4)
  {
    this->template createInternal<false, false>({n0, n1, n2, n3, n4});
  }
  void create(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4)
  {
    this->template createInternal<true, true>({n0, n1, n2, n3, n4});
  }
  void createHost(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4)
  {
    this->template createInternal<true, false>({n0, n1, n2, n3, n4});
  }
  void createDevice(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4)
  {
    this->template createInternal<false, true>({n0, n1, n2, n3, n4});
  }
  void offset(int64_t d0, int64_t d1, int64_t d2, int64_t d3, int64_t d4)
  {
    ArrayBase<T, 5>::offset({d0, d1, d2, d3, d4});
  }

  KOKKOS_FUNCTION T & operator()(int64_t i0, int64_t i1, int64_t i2, int64_t i3, int64_t i4) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && i0 - _d[0] < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && i1 - _d[1] < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && i2 - _d[2] < _n[2]);
    KOKKOS_ASSERT(i3 - _d[3] >= 0 && i3 - _d[3] < _n[3]);
    KOKKOS_ASSERT(i4 - _d[4] >= 0 && i4 - _d[4] < _n[4]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1] + (i2 - _d[2]) * _s[2] +
                            (i3 - _d[3]) * _s[3] + (i4 - _d[4]) * _s[4]);
  }
#endif
};

template <typename T>
using Array1D = Array<T, 1>;
template <typename T>
using Array2D = Array<T, 2>;
template <typename T>
using Array3D = Array<T, 3>;
template <typename T>
using Array4D = Array<T, 4>;
template <typename T>
using Array5D = Array<T, 5>;

} // namespace Kokkos
} // namespace Moose
