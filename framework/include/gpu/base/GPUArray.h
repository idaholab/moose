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
  using ArrayBase<T, dimension>::operator=

namespace Moose
{
namespace Kokkos
{

// This function simply calls ::Kokkos::kokkos_free, but it is separately defined in KokkosArray.K
// because the Kokkos function cannot be directly seen by the host compiler
void kokkosFree(void * ptr);

/**
 * The enumerator that dictates the memory copy direction
 */
enum class MemcpyKind
{
  HOST_TO_HOST,
  HOST_TO_DEVICE,
  DEVICE_TO_HOST,
  DEVICE_TO_DEVICE
};

/**
 * The Kokkos array class
 */
template <typename T, unsigned int dimension = 1>
class Array;

/**
 * The type trait that determines the default behavior of copy constructor and deepCopy()
 * If this type trait is set to true, the copy constructor will call deepCopy(),
 * and the deepCopy() method will copy-construct each entry.
 * If this type trait is set to false, the copy constructor will call shallowCopy(),
 * and the deepCopy() method will do a memory copy.
 */
///@{
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
///@}

/**
 * The base class for Kokkos arrays
 */
template <typename T, unsigned int dimension>
class ArrayBase
{
public:
  /**
   * Default constructor
   */
  ArrayBase() = default;

  /**
   * Copy constructor
   */
  ArrayBase(const ArrayBase<T, dimension> & array)
  {
#ifndef MOOSE_KOKKOS_SCOPE
    static_assert(!ArrayDeepCopy<T>::value,
                  "Kokkos array cannot be deep copied outside the Kokkos compilation scope");
#endif

    if constexpr (ArrayDeepCopy<T>::value)
      deepCopy(array);
    else
      shallowCopy(array);
  }

  /**
   * Destructor
   */
  ~ArrayBase() { destroy(); }

  /**
   * Free all data and reset
   */
  void destroy();

  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  void shallowCopy(const ArrayBase<T, dimension> & array);

  /**
   * Get the reference count
   * @returns The reference count
   */
  unsigned int useCount() const { return _counter.use_count(); }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether the array was allocated either on host or device
   * @returns Whether the array was allocated either on host or device
   */
  KOKKOS_FUNCTION bool isAlloc() const { return _is_host_alloc || _is_device_alloc; }
  /**
   * Get whether the array was allocated on host
   * @returns Whether the array was allocated on host
   */
  KOKKOS_FUNCTION bool isHostAlloc() const { return _is_host_alloc; }
  /**
   * Get whether the array was allocated on device
   * @returns Whether the array was allocated on device
   */
  KOKKOS_FUNCTION bool isDeviceAlloc() const { return _is_device_alloc; }
  /**
   * Get whether the host array was aliased
   * @returns Whether the host array was aliased
   */
  KOKKOS_FUNCTION bool isHostAlias() const { return _is_host_alias; }
  /**
   * Get whether the device array was aliased
   * @returns Whether the device array was aliased
   */
  KOKKOS_FUNCTION bool isDeviceAlias() const { return _is_device_alias; }
  /**
   * Get the total array size
   * @returns The total array size
   */
  KOKKOS_FUNCTION dof_id_type size() const { return _size; }
  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The array size of the dimension
   */
  KOKKOS_FUNCTION dof_id_type n(unsigned int dim) const { return _n[dim]; }
  /**
   * Get the data pointer
   * @returns The pointer to the underlying data depending on the architecture this function is
   * being called on
   */
  KOKKOS_FUNCTION T * data() const
  {
    KOKKOS_IF_ON_HOST(return _host_data;)

    return _device_data;
  }
  /**
   * Get the first element
   * @returns The reference of the first element depending on the architecture this function is
   * being called on
   */
  KOKKOS_FUNCTION T & first() const
  {
    KOKKOS_IF_ON_HOST(return _host_data[0];)

    return _device_data[0];
  }
  /**
   * Get the last element
   * @returns The reference of the last element depending on the architecture this function is being
   * called on
   */
  KOKKOS_FUNCTION T & last() const
  {
    KOKKOS_IF_ON_HOST(return _host_data[_size - 1];)

    return _device_data[_size - 1];
  }
  /**
   * Get an array entry
   * @param i The dimensionless index
   * @returns The reference of the entry depending on the architecture this function is being called
   * on
   */
  KOKKOS_FUNCTION T & operator[](dof_id_type i) const
  {
    KOKKOS_ASSERT(i < _size);

    KOKKOS_IF_ON_HOST(return _host_data[i];)

    return _device_data[i];
  }

  /**
   * Get the host data pointer
   * @returns The pointer to the underlying host data
   */
  T * hostData() const { return _host_data; }
  /**
   * Get the device data pointer
   * @returns The pointer to the underlying device data
   */
  T * deviceData() const { return _device_data; }
  /**
   * Allocate array on host and device
   * @param n The vector containing the size of each dimension
   */
  void create(const std::vector<dof_id_type> & n) { createInternal<true, true>(n); }
  /**
   * Allocate array on host only
   * @param n The vector containing the size of each dimension
   */
  void createHost(const std::vector<dof_id_type> & n) { createInternal<true, false>(n); }
  /**
   * Allocate array on device only
   * @param n The vector containing the size of each dimension
   */
  void createDevice(const std::vector<dof_id_type> & n) { createInternal<false, true>(n); }
  /**
   * Point the host data to an external data instead of allocating it
   * @param ptr The pointer to the external host data
   */
  void aliasHost(T * ptr);
  /**
   * Point the device data to an external data instead of allocating it
   * @param ptr The pointer to the external device data
   */
  void aliasDevice(T * ptr);
  /**
   * Apply starting index offsets to each dimension
   * @param d The vector containing the offset of each dimension
   */
  void offset(const std::vector<dof_id_signed_type> & d);
  /**
   * Copy data from host to device
   */
  void copyToDevice();
  /**
   * Copy data from device to host
   */
  void copyToHost();
  /**
   * Copy data from an external data to this array
   * @param ptr The pointer to the external data
   * @param dir The copy direction
   * @param n The number of entries to copy
   * @param offset The starting offset of this array
   */
  void copyIn(const T * ptr, MemcpyKind dir, dof_id_type n, dof_id_type offset = 0);
  /**
   * Copy data to an external data from this array
   * @param ptr The pointer to the external data
   * @param dir The copy direction
   * @param n The number of entries to copy
   * @param offset The starting offset of this array
   */
  void copyOut(T * ptr, MemcpyKind dir, dof_id_type n, dof_id_type offset = 0);
  /**
   * Copy all the nested Kokkos arrays including self from host to device
   */
  void copyToDeviceNested();
  /**
   * Deep copy another Kokkos array
   * If ArrayDeepCopy<T>::value is true, it will copy-construct each entry
   * If ArrayDeepCopy<T>::value is false, it will do a memory copy
   * @param array The Kokkos array to be deep copied
   */
  void deepCopy(const ArrayBase<T, dimension> & array);
  /**
   * Swap with another Kokkos array
   * @param array The Kokkos array to be swapped
   */
  void swap(ArrayBase<T, dimension> & array);

  /**
   * Assign a scalar value uniformly
   * @param scalar The scalar value to be assigned
   */
  auto & operator=(const T & scalar);

  /**
   * Array iterator
   */
  class iterator
  {
  public:
    KOKKOS_FUNCTION iterator(T * it) : it(it) {}
    KOKKOS_FUNCTION bool operator==(const iterator & other) const { return it == other.it; }
    KOKKOS_FUNCTION bool operator!=(const iterator & other) const { return it != other.it; }
    KOKKOS_FUNCTION T & operator*() const { return *it; }
    KOKKOS_FUNCTION T * operator&() const { return it; }
    KOKKOS_FUNCTION iterator & operator++()
    {
      ++it;
      return *this;
    }
    KOKKOS_FUNCTION iterator operator++(int)
    {
      iterator pre = *this;
      ++it;
      return pre;
    }

  private:
    T * it;
  };

  /**
   * Get the beginning iterator
   * @returns The beginning iterator
   */
  KOKKOS_FUNCTION iterator begin() const
  {
    KOKKOS_IF_ON_HOST(return iterator(_host_data);)

    return iterator(_device_data);
  }
  /**
   * Get the end iterator
   * @returns The end iterator
   */
  KOKKOS_FUNCTION iterator end() const
  {
    KOKKOS_IF_ON_HOST(return iterator(_host_data + _size);)

    return iterator(_device_data + _size);
  }
#endif

protected:
  /**
   * Size of each dimension
   */
  dof_id_type _n[dimension] = {0};
  /**
   * Stride of each dimension
   */
  dof_id_type _s[dimension] = {0};
  /**
   * Offset of each dimension
   */
  dof_id_signed_type _d[dimension] = {0};

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * The internal method to initialize and allocate this array
   * @tparam host Whether host data will be allocated
   * @tparam device Whether device data will be allocated
   * @param n The vector containing the size of each dimension
   */
  template <bool host, bool device>
  void createInternal(const std::vector<dof_id_type> & n);
  /**
   * The internal method to initialize and allocate this array
   * @param n The vector containing the size of each dimension
   * @param host The flag whether host data will be allocated
   * @param device The flag whether device data will be allocated
   */
  void createInternal(const std::vector<dof_id_type> & n, bool host, bool device);
  /**
   * The internal method to perform a memory copy
   * @tparam TargetSpace The Kokkos memory space of target data
   * @tparam Sourcespace The Kokkos memory space of source data
   * @param target The pointer to the target data
   * @param source The pointer to the source data
   * @param n The number of entries to copy
   */
  template <typename TargetSpace, typename SourceSpace>
  void copyInternal(T * target, const T * source, dof_id_type n);
#endif

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Allocate host data for an initialized array that has not allocated host data
   */
  void allocHost();
  /**
   * Allocate device data for an initialized array that has not allocated device data
   */
  void allocDevice();
#endif

  /**
   * Reference counter
   */
  std::shared_ptr<unsigned int> _counter;
  /**
   * Flag whether host data was allocated
   */
  bool _is_host_alloc = false;
  /**
   * Flag whether device data was allocated
   */
  bool _is_device_alloc = false;
  /**
   * Flag whether the host data points to an external data
   */
  bool _is_host_alias = false;
  /**
   * Flag whether the device data points to an external data
   */
  bool _is_device_alias = false;
  /**
   * Host data
   */
  T * _host_data = nullptr;
  /**
   * Device data
   */
  T * _device_data = nullptr;
  /**
   * Total size
   */
  dof_id_type _size = 0;
};

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::destroy()
{
  if (!_counter)
    return;

  if (_counter.use_count() > 1)
  {
    _host_data = nullptr;
    _device_data = nullptr;
  }
  else if (_counter.use_count() == 1)
  {
    if (_is_host_alloc && !_is_host_alias)
    {
      if constexpr (std::is_default_constructible<T>::value)
        // Allocated by new
        delete[] _host_data;
      else
      {
        // Allocated by malloc
        for (dof_id_type i = 0; i < _size; ++i)
          _host_data[i].~T();

        std::free(_host_data);
      }
    }

    if (_is_device_alloc && !_is_device_alias)
      kokkosFree(_device_data);
  }

  _size = 0;

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _n[i] = 0;
    _s[i] = 0;
    _d[i] = 0;
  }

  _is_host_alloc = false;
  _is_device_alloc = false;
  _is_host_alias = false;
  _is_device_alias = false;

  _counter.reset();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::shallowCopy(const ArrayBase<T, dimension> & array)
{
  destroy();

  _counter = array._counter;

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

  _device_data =
      static_cast<T *>(::Kokkos::kokkos_malloc<ExecSpace::memory_space>(_size * sizeof(T)));

  _is_device_alloc = true;
}

template <typename T, unsigned int dimension>
template <bool host, bool device>
void
ArrayBase<T, dimension>::createInternal(const std::vector<dof_id_type> & n)
{
  if (n.size() != dimension)
    mooseError("Kokkos array error: the number of dimensions provided (",
               n.size(),
               ") must match the array dimension (",
               dimension,
               ").");

  if (_counter)
    destroy();

  _counter = std::make_shared<unsigned int>();

  _size = 1;
  _s[0] = 1;

  for (const auto i : make_range(dimension))
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
ArrayBase<T, dimension>::createInternal(const std::vector<dof_id_type> & n, bool host, bool device)
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
ArrayBase<T, dimension>::copyInternal(T * target, const T * source, dof_id_type n)
{
  ::Kokkos::Impl::DeepCopy<TargetSpace, SourceSpace>(target, source, n * sizeof(T));
  ::Kokkos::fence();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::offset(const std::vector<dof_id_signed_type> & d)
{
  if (d.size() > dimension)
    mooseError("Kokkos array error: the number of offsets provided (",
               d.size(),
               ") cannot be larger than the array dimension (",
               dimension,
               ").");

  for (const auto i : index_range(d))
    _d[i] = d[i];
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyToDevice()
{
  // If host side memory is not allocated, do nothing
  if (!_is_host_alloc)
    return;

  // If device side memory is not allocated,
  if (!_is_device_alloc)
  {
    if (_counter.use_count() == 1)
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

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyToHost()
{
  // If device side memory is not allocated, do nothing
  if (!_is_device_alloc)
    return;

  // If host side memory is not allocated,
  if (!_is_host_alloc)
  {
    if (_counter.use_count() == 1)
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

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyIn(const T * ptr, MemcpyKind dir, dof_id_type n, dof_id_type offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copyin data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyKind::HOST_TO_HOST)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from host to host
    copyInternal<::Kokkos::HostSpace, ::Kokkos::HostSpace>(_host_data + offset, ptr, n);
  }
  else if (dir == MemcpyKind::HOST_TO_DEVICE)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(_device_data + offset, ptr, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_HOST)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(_host_data + offset, ptr, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_DEVICE)
  {
    // If device side memory is not allocated, do nothing
    if (!_is_device_alloc)
      return;

    // Copy from device to device
    copyInternal<MemSpace, MemSpace>(_device_data + offset, ptr, n);
  }
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyOut(T * ptr, MemcpyKind dir, dof_id_type n, dof_id_type offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copyout data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyKind::HOST_TO_HOST)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from host to host
    copyInternal<::Kokkos::HostSpace, ::Kokkos::HostSpace>(ptr, _host_data + offset, n);
  }
  else if (dir == MemcpyKind::HOST_TO_DEVICE)
  {
    // If host side memory is not allocated, do nothing
    if (!_is_host_alloc)
      return;

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(ptr, _host_data + offset, n);
  }
  else if (dir == MemcpyKind::DEVICE_TO_HOST)
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
}

template <typename T>
void
copyToDeviceInner(T & /* data */)
{
}

template <typename T, unsigned int dimension>
void
copyToDeviceInner(Array<T, dimension> & data)
{
  data.copyToDeviceNested();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::copyToDeviceNested()
{
  for (unsigned int i = 0; i < _size; ++i)
    copyToDeviceInner(_host_data[i]);

  copyToDevice();
}

template <typename T, unsigned int dimension>
void
ArrayBase<T, dimension>::deepCopy(const ArrayBase<T, dimension> & array)
{
  if (ArrayDeepCopy<T>::value && !array._is_host_alloc)
    mooseError(
        "Kokkos array error: cannot deep copy using constructor from array without host data.");

  std::vector<dof_id_type> n(std::begin(array._n), std::end(array._n));

  createInternal(n, array._is_host_alloc, array._is_device_alloc);

  if constexpr (ArrayDeepCopy<T>::value)
  {
    for (dof_id_type i = 0; i < _size; ++i)
      new (_host_data + i) T(array._host_data[i]);

    copyToDevice();
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
    ::Kokkos::Experimental::fill_n(ExecSpace(), data, _size, scalar);
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

    for (dof_id_type i = 0; i < array.size(); ++i)
      dataStore(stream, data[i], context);

    std::free(data);
  }
  else
    for (auto & value : array)
      dataStore(stream, value, context);
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

  std::vector<dof_id_type> from_n(dimension);
  std::vector<dof_id_type> n(dimension);

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
      array.copyToDevice();
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

/**
 * The specialization of the Kokkos array class for each dimension.
 * All array data that needs to be accessed on device in Kokkos objects should use this class.
 * If the array is populated on host and is to be accessed on device, make sure to call
 * copyToDevice() after populating data. For a nested Kokkos array, either copyToDeviceNested()
 * should be called for the outermost array or copyToDevice() should be called for each instance of
 * Kokkos array from the innermost to the outermost. Do not store this object as reference in your
 * Kokkos object if it used on device, because the reference refers to a host object and therefore
 * is not accessible on device. If storing it as a reference is required, see ReferenceWrapper.
 */
///@{
template <typename T>
class Array<T, 1> : public ArrayBase<T, 1>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 1);
#endif

public:
  /**
   * Default constructor
   */
  Array() = default;
  /**
   * Copy constructor
   */
  Array(const Array<T, 1> & array) : ArrayBase<T, 1>(array) {}
  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 1> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n0 The first dimension size
   */
  Array(dof_id_type n0) { create(n0); }
  /**
   * Constructor
   * Initialize and allocate array by copying a standard vector variable
   * This allocates and copies to both host and device data
   * @param vector The standard vector variable to copy
   */
  Array(const std::vector<T> & vector) { *this = vector; }

  /**
   * Initialize array with given dimensions but do not allocate
   * @param n0 The first dimension size
   */
  void init(dof_id_type n0) { this->template createInternal<false, false>({n0}); }
  /**
   * Initialize and allocate array with given dimensions on host and device
   * @param n0 The first dimension size
   */
  void create(dof_id_type n0) { this->template createInternal<true, true>({n0}); }
  /**
   * Initialize and allocate array with given dimensions on host only
   * @param n0 The first dimension size
   */
  void createHost(dof_id_type n0) { this->template createInternal<true, false>({n0}); }
  /**
   * Initialize and allocate array with given dimensions on device only
   * @param n0 The first dimension size
   */
  void createDevice(dof_id_type n0) { this->template createInternal<false, true>({n0}); }
  /**
   * Set starting index offsets
   * @param d0 The first dimension offset
   */
  void offset(dof_id_signed_type d0) { ArrayBase<T, 1>::offset({d0}); }

  /**
   * Copy a standard vector variable
   * This re-initializes and re-allocates array with the size of the vector
   * @tparam host Whether to allocate and copy to the host data
   * @tparam device Whether to allocate and copy to the device data
   * @param vector The standard vector variable to copy
   */
  template <bool host, bool device>
  void copyVector(const std::vector<T> & vector)
  {
    this->template createInternal<host, device>({static_cast<dof_id_type>(vector.size())});

    if (host)
      std::memcpy(this->hostData(), vector.data(), this->size() * sizeof(T));

    if (device)
      this->template copyInternal<MemSpace, ::Kokkos::HostSpace>(
          this->deviceData(), vector.data(), this->size());
  }
  /**
   * Copy a standard set variable
   * This re-initializes and re-allocates array with the size of the set
   * @tparam host Whether to allocate and copy to the host data
   * @tparam device Whether to allocate and copy to the device data
   * @param set The standard set variable to copy
   */
  template <bool host, bool device>
  void copySet(const std::set<T> & set)
  {
    std::vector<T> vector(set.begin(), set.end());

    copyVector<host, device>(vector);
  }

  /**
   * Copy a standard vector variable
   * This allocates and copies to both host and device data
   * @param vector The standard vector variable to copy
   */
  auto & operator=(const std::vector<T> & vector)
  {
    copyVector<true, true>(vector);

    return *this;
  }
  /**
   * Copy a standard set variable
   * This allocates and copies to both host and device data
   * @param set The standard set variable to copy
   */
  auto & operator=(const std::set<T> & set)
  {
    copySet<true, true>(set);

    return *this;
  }

  /**
   * Get an array entry
   * @param i0 The first dimension index
   * @returns The reference of the entry depending on the architecture this function is being
   * called on
   */
  KOKKOS_FUNCTION T & operator()(dof_id_signed_type i0) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && static_cast<dof_id_type>(i0 - _d[0]) < _n[0]);

    return this->operator[](i0 - _d[0]);
  }
#endif
};

template <typename T>
class Array<T, 2> : public ArrayBase<T, 2>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 2);
#endif

public:
  /**
   * Default constructor
   */
  Array() = default;
  /**
   * Copy constructor
   */
  Array(const Array<T, 2> & array) : ArrayBase<T, 2>(array) {}
  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 2> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   */
  Array(dof_id_type n0, dof_id_type n1) { create(n0, n1); }

  /**
   * Initialize array with given dimensions but do not allocate
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   */
  void init(dof_id_type n0, dof_id_type n1)
  {
    this->template createInternal<false, false>({n0, n1});
  }
  /**
   * Initialize and allocate array with given dimensions on host and device
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   */
  void create(dof_id_type n0, dof_id_type n1)
  {
    this->template createInternal<true, true>({n0, n1});
  }
  /**
   * Initialize and allocate array with given dimensions on host only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   */
  void createHost(dof_id_type n0, dof_id_type n1)
  {
    this->template createInternal<true, false>({n0, n1});
  }
  /**
   * Initialize and allocate array with given dimensions on device only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   */
  void createDevice(dof_id_type n0, dof_id_type n1)
  {
    this->template createInternal<false, true>({n0, n1});
  }
  /**
   * Set starting index offsets
   * @param d0 The first dimension offset
   * @param d1 The second dimension offset
   */
  void offset(dof_id_signed_type d0, dof_id_signed_type d1) { ArrayBase<T, 2>::offset({d0, d1}); }

  /**
   * Get an array entry
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @returns The reference of the entry depending on the architecture this function is being called
   * on
   */
  KOKKOS_FUNCTION T & operator()(dof_id_signed_type i0, dof_id_signed_type i1) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && static_cast<dof_id_type>(i0 - _d[0]) < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && static_cast<dof_id_type>(i1 - _d[1]) < _n[1]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1]);
  }
#endif
};

template <typename T>
class Array<T, 3> : public ArrayBase<T, 3>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 3);
#endif

public:
  /**
   * Default constructor
   */
  Array() = default;
  /**
   * Copy constructor
   */
  Array(const Array<T, 3> & array) : ArrayBase<T, 3>(array) {}
  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 3> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   */
  Array(dof_id_type n0, dof_id_type n1, dof_id_type n2) { create(n0, n1, n2); }

  /**
   * Initialize array with given dimensions but do not allocate
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   */
  void init(dof_id_type n0, dof_id_type n1, dof_id_type n2)
  {
    this->template createInternal<false, false>({n0, n1, n2});
  }
  /**
   * Initialize and allocate array with given dimensions on host and device
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   */
  void create(dof_id_type n0, dof_id_type n1, dof_id_type n2)
  {
    this->template createInternal<true, true>({n0, n1, n2});
  }
  /**
   * Initialize and allocate array with given dimensions on host only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   */
  void createHost(dof_id_type n0, dof_id_type n1, dof_id_type n2)
  {
    this->template createInternal<true, false>({n0, n1, n2});
  }
  /**
   * Initialize and allocate array with given dimensions on device only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   */
  void createDevice(dof_id_type n0, dof_id_type n1, dof_id_type n2)
  {
    this->template createInternal<false, true>({n0, n1, n2});
  }
  /**
   * Set starting index offsets
   * @param d0 The first dimension offset
   * @param d1 The second dimension offset
   * @param d2 The third dimension offset
   */
  void offset(dof_id_signed_type d0, dof_id_signed_type d1, dof_id_signed_type d2)
  {
    ArrayBase<T, 3>::offset({d0, d1, d2});
  }

  /**
   * Get an array entry
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @returns The reference of the entry depending on the architecture this function is being
   * called on
   */
  KOKKOS_FUNCTION T &
  operator()(dof_id_signed_type i0, dof_id_signed_type i1, dof_id_signed_type i2) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && static_cast<dof_id_type>(i0 - _d[0]) < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && static_cast<dof_id_type>(i1 - _d[1]) < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && static_cast<dof_id_type>(i2 - _d[2]) < _n[2]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1] + (i2 - _d[2]) * _s[2]);
  }
#endif
};

template <typename T>
class Array<T, 4> : public ArrayBase<T, 4>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 4);
#endif

public:
  /**
   * Default constructor
   */
  Array() = default;
  /**
   * Copy constructor
   */
  Array(const Array<T, 4> & array) : ArrayBase<T, 4>(array) {}
  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 4> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   */
  Array(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3) { create(n0, n1, n2, n3); }

  /**
   * Initialize array with given dimensions but do not allocate
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   */
  void init(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3)
  {
    this->template createInternal<false, false>({n0, n1, n2, n3});
  }
  /**
   * Initialize and allocate array with given dimensions on host and device
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   */
  void create(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3)
  {
    this->template createInternal<true, true>({n0, n1, n2, n3});
  }
  /**
   * Initialize and allocate array with given dimensions on host only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   */
  void createHost(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3)
  {
    this->template createInternal<true, false>({n0, n1, n2, n3});
  }
  /**
   * Initialize and allocate array with given dimensions on device only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   */
  void createDevice(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3)
  {
    this->template createInternal<false, true>({n0, n1, n2, n3});
  }
  /**
   * Set starting index offsets
   * @param d0 The first dimension offset
   * @param d1 The second dimension offset
   * @param d2 The third dimension offset
   * @param d3 The fourth dimension offset
   */
  void
  offset(dof_id_signed_type d0, dof_id_signed_type d1, dof_id_signed_type d2, dof_id_signed_type d3)
  {
    ArrayBase<T, 4>::offset({d0, d1, d2, d3});
  }

  /**
   * Get an array entry
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @param i3 The fourth dimension index
   * @returns The reference of the entry depending on the architecture this function is being called
   * on
   */
  KOKKOS_FUNCTION T & operator()(dof_id_signed_type i0,
                                 dof_id_signed_type i1,
                                 dof_id_signed_type i2,
                                 dof_id_signed_type i3) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && static_cast<dof_id_type>(i0 - _d[0]) < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && static_cast<dof_id_type>(i1 - _d[1]) < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && static_cast<dof_id_type>(i2 - _d[2]) < _n[2]);
    KOKKOS_ASSERT(i3 - _d[3] >= 0 && static_cast<dof_id_type>(i3 - _d[3]) < _n[3]);

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
#endif

public:
  /**
   * Default constructor
   */
  Array() = default;
  /**
   * Copy constructor
   */
  Array(const Array<T, 5> & array) : ArrayBase<T, 5>(array) {}
  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 5> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   * @param n4 The fifth dimension size
   */
  Array(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3, dof_id_type n4)
  {
    create(n0, n1, n2, n3, n4);
  }

  /**
   * Initialize array with given dimensions but do not allocate
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   * @param n4 The fifth dimension size
   */
  void init(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3, dof_id_type n4)
  {
    this->template createInternal<false, false>({n0, n1, n2, n3, n4});
  }
  /**
   * Initialize and allocate array with given dimensions on host and device
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   * @param n4 The fifth dimension size
   */
  void create(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3, dof_id_type n4)
  {
    this->template createInternal<true, true>({n0, n1, n2, n3, n4});
  }
  /**
   * Initialize and allocate array with given dimensions on host only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   * @param n4 The fifth dimension size
   */
  void createHost(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3, dof_id_type n4)
  {
    this->template createInternal<true, false>({n0, n1, n2, n3, n4});
  }
  /**
   * Initialize and allocate array with given dimensions on device only
   * @param n0 The first dimension size
   * @param n1 The second dimension size
   * @param n2 The third dimension size
   * @param n3 The fourth dimension size
   * @param n4 The fifth dimension size
   */
  void createDevice(dof_id_type n0, dof_id_type n1, dof_id_type n2, dof_id_type n3, dof_id_type n4)
  {
    this->template createInternal<false, true>({n0, n1, n2, n3, n4});
  }
  /**
   * Set starting index offsets
   * @param d0 The first dimension offset
   * @param d1 The second dimension offset
   * @param d2 The third dimension offset
   * @param d3 The fourth dimension offset
   * @param d4 The fifth dimension offset
   */
  void offset(dof_id_signed_type d0,
              dof_id_signed_type d1,
              dof_id_signed_type d2,
              dof_id_signed_type d3,
              dof_id_signed_type d4)
  {
    ArrayBase<T, 5>::offset({d0, d1, d2, d3, d4});
  }

  /**
   * Get an array entry
   * @param i0 The first dimension index
   * @param i1 The second dimension index
   * @param i2 The third dimension index
   * @param i3 The fourth dimension index
   * @param i4 The fifth dimension index
   * @returns The reference of the entry depending on the architecture this function is being called
   * on
   */
  KOKKOS_FUNCTION T & operator()(dof_id_signed_type i0,
                                 dof_id_signed_type i1,
                                 dof_id_signed_type i2,
                                 dof_id_signed_type i3,
                                 dof_id_signed_type i4) const
  {
    KOKKOS_ASSERT(i0 - _d[0] >= 0 && static_cast<dof_id_type>(i0 - _d[0]) < _n[0]);
    KOKKOS_ASSERT(i1 - _d[1] >= 0 && static_cast<dof_id_type>(i1 - _d[1]) < _n[1]);
    KOKKOS_ASSERT(i2 - _d[2] >= 0 && static_cast<dof_id_type>(i2 - _d[2]) < _n[2]);
    KOKKOS_ASSERT(i3 - _d[3] >= 0 && static_cast<dof_id_type>(i3 - _d[3]) < _n[3]);
    KOKKOS_ASSERT(i4 - _d[4] >= 0 && static_cast<dof_id_type>(i4 - _d[4]) < _n[4]);

    return this->operator[](i0 - _d[0] + (i1 - _d[1]) * _s[1] + (i2 - _d[2]) * _s[2] +
                            (i3 - _d[3]) * _s[3] + (i4 - _d[4]) * _s[4]);
  }
#endif
};
///@}

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
