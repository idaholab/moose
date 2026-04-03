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
#include "KokkosHeader.h"
#endif

#include "Conversion.h"
#include "DataIO.h"

#define usingKokkosArrayBaseMembers(T, dimension, index_type)                                      \
private:                                                                                           \
  using ArrayBase<T, dimension, index_type>::_n;                                                   \
  using ArrayBase<T, dimension, index_type>::_s;                                                   \
  using ArrayBase<T, dimension, index_type>::_d;                                                   \
  using ArrayBase<T, dimension, index_type>::_is_offset;                                           \
                                                                                                   \
public:                                                                                            \
  using typename ArrayBase<T, dimension, index_type>::signed_index_type;                           \
  using ArrayBase<T, dimension, index_type>::operator=

namespace Moose::Kokkos
{

// This function simply calls ::Kokkos::kokkos_free, but it is separately defined in KokkosArray.K
// because the Kokkos function cannot be directly seen by the host compiler
void free(void * ptr);

/**
 * The enumerator that dictates the memory copy direction
 */
enum class MemcpyType
{
  HOST_TO_HOST,
  HOST_TO_DEVICE,
  DEVICE_TO_HOST,
  DEVICE_TO_DEVICE
};

/**
 * The enumerator that dictates the memory layout
 */
enum class LayoutType
{
  LEFT,
  RIGHT
};

/**
 * The Kokkos array class
 */
template <typename T,
          unsigned int dimension = 1,
          typename index_type = dof_id_type,
          LayoutType layout = LayoutType::LEFT>
class Array;

/**
 * The type trait that determines if a template type is Kokkos array
 */
///@{
template <typename>
struct is_kokkos_array : std::false_type
{
};

template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
struct is_kokkos_array<Array<T, dimension, index_type, layout>> : std::true_type
{
};
///@}

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
  static constexpr bool value = false;
};

template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
struct ArrayDeepCopy<Array<T, dimension, index_type, layout>>
{
  static constexpr bool value = ArrayDeepCopy<T>::value;
};
///@}

/**
 * The base class for Kokkos arrays
 */
template <typename T, unsigned int dimension, typename index_type>
class ArrayBase
{
  static_assert(std::is_integral_v<index_type>, "Kokkos array index type must be an integral type");
  static_assert(std::is_unsigned_v<index_type>, "Kokkos array index type must be unsigned");
  static_assert(!std::is_same_v<bool, index_type>, "Kokkos array index type must not be bool");

public:
  using signed_index_type = typename std::make_signed<index_type>::type;

  /**
   * Constructor
   * @param layout The memory layout type
   */
  ArrayBase(const LayoutType layout) : _layout(layout) {}

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param layout The memory layout type
   * @param n The size of each dimension
   */
  template <typename... size_type>
  ArrayBase(const LayoutType layout, size_type... n) : _layout(layout)
  {
    create(n...);
  }
#endif

  /**
   * Copy constructor
   */
  ArrayBase(const ArrayBase<T, dimension, index_type> & array) : _layout(array._layout)
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
  void shallowCopy(const ArrayBase<T, dimension, index_type> & array);

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
  KOKKOS_FUNCTION index_type size() const { return _size; }
  /**
   * Get the size of a dimension
   * @param dim The dimension index
   * @returns The size of the dimension
   */
  KOKKOS_FUNCTION index_type n(unsigned int dim) const { return _n[dim]; }
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
  KOKKOS_FUNCTION T & operator[](index_type i) const
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
   * Get the host unmanaged view
   * @returns The host unmanaged view
   */
  auto hostView() const
  {
    return ::Kokkos::View<T *, ::Kokkos::HostSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>>(
        _host_data, _size);
  }
  /**
   * Get the device unmanaged view
   * @returns The device unmanaged view
   */
  auto deviceView() const
  {
    return ::Kokkos::View<T *, MemSpace, ::Kokkos::MemoryTraits<::Kokkos::Unmanaged>>(_device_data,
                                                                                      _size);
  }
  /**
   * Initialize array with given dimensions but do not allocate
   * @param n The size of each dimension
   */
  template <typename... size_type>
  void init(size_type... n)
  {
    createInternal<false, false, false>(n...);
  }
  /**
   * Allocate array on host and device
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The vector containing the size of each dimension
   */
  template <bool initialize = true>
  void create(const std::vector<index_type> & n)
  {
    createInternal<true, true, initialize>(n);
  }
  /**
   * Allocate array on host and device
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The size of each dimension
   */
  template <bool initialize = true, typename... size_type>
  void create(size_type... n)
  {
    createInternal<true, true, initialize>(n...);
  }
  /**
   * Allocate array on host only
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The vector containing the size of each dimension
   */
  template <bool initialize = true>
  void createHost(const std::vector<index_type> & n)
  {
    createInternal<true, false, initialize>(n);
  }
  /**
   * Allocate array on host only
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The size of each dimension
   */
  template <bool initialize = true, typename... size_type>
  void createHost(size_type... n)
  {
    createInternal<true, false, initialize>(n...);
  }
  /**
   * Allocate array on device only
   * @param n The vector containing the size of each dimension
   */
  void createDevice(const std::vector<index_type> & n) { createInternal<false, true, false>(n); }
  /**
   * Allocate array on device only
   * @param n The size of each dimension
   */
  template <typename... size_type>
  void createDevice(size_type... n)
  {
    createInternal<false, true, false>(n...);
  }
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
  void offset(const std::vector<signed_index_type> & d);
  /**
   * Apply starting index offsets to each dimension
   * @param d The offset of each dimension
   */
  template <typename... offset_type>
  void offset(offset_type... d);
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
  void copyIn(const T * ptr, MemcpyType dir, index_type n, index_type offset = 0);
  /**
   * Copy data to an external data from this array
   * @param ptr The pointer to the external data
   * @param dir The copy direction
   * @param n The number of entries to copy
   * @param offset The starting offset of this array
   */
  void copyOut(T * ptr, MemcpyType dir, index_type n, index_type offset = 0);
  /**
   * Copy all the nested Kokkos arrays including self from host to device
   */
  void copyToDeviceNested();
  /**
   * Copy data from host to device and deallocate host
   * @param should_free_host Whether the host memory should be freed.
   * Host memory cannot be freed when there are shallow copies of this array that are still alive.
   * If \p should_free_host is true, and we cannot free for above reason, it will error.
   */
  void moveToDevice(bool should_free_host = true);
  /**
   * Copy data from device to host and deallocate device
   * @param should_free_device Whether the device memory should be freed.
   * Device memory cannot be freed when there are shallow copies of this array that are still alive.
   * If \p should_free_device is true, and we cannot free for above reason, it will error.
   */
  void moveToHost(bool should_free_device = true);
  /**
   * Deep copy another Kokkos array
   * If ArrayDeepCopy<T>::value is true, it will copy-construct each entry
   * If ArrayDeepCopy<T>::value is false, it will do a memory copy
   * @param array The Kokkos array to be deep copied
   */
  void deepCopy(const ArrayBase<T, dimension, index_type> & array);
  /**
   * Swap with another Kokkos array
   * @param array The Kokkos array to be swapped
   */
  void swap(ArrayBase<T, dimension, index_type> & array);

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
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    KOKKOS_FUNCTION iterator() : it(nullptr) {}
    KOKKOS_FUNCTION explicit iterator(T * p) : it(p) {}

    KOKKOS_FUNCTION reference operator*() const { return *it; }
    KOKKOS_FUNCTION pointer operator->() const { return it; }
    KOKKOS_FUNCTION pointer operator&() const { return it; }
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
    KOKKOS_FUNCTION friend bool operator==(const iterator & a, const iterator & b)
    {
      return a.it == b.it;
    }
    KOKKOS_FUNCTION friend bool operator!=(const iterator & a, const iterator & b)
    {
      return a.it != b.it;
    }

  private:
    pointer it;
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
  index_type _n[dimension] = {0};
  /**
   * Stride of each dimension
   */
  index_type _s[dimension] = {0};
  /**
   * Offset of each dimension
   */
  signed_index_type _d[dimension] = {0};
  /**
   * Flag whether the array indices are offset
   */
  bool _is_offset = false;
  /**
   * Flag whether host data was allocated using malloc
   */
  bool _is_malloc = false;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * The internal method to initialize and allocate this array
   * @tparam host Whether host data will be allocated
   * @tparam device Whether device data will be allocated
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The size of each dimension
   */
  template <bool host, bool device, bool initialize, typename... size_type>
  void createInternal(size_type... n);
  /**
   * The internal method to initialize and allocate this array
   * @tparam host Whether host data will be allocated
   * @tparam device Whether device data will be allocated
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The vector containing the size of each dimension
   */
  template <bool host, bool device, bool initialize>
  void createInternal(const std::vector<index_type> & n);
  /**
   * The internal method to initialize and allocate this array
   * @tparam initialize Whether to initialize host data (calls default constructor)
   * @param n The vector containing the size of each dimension
   * @param host The flag whether host data will be allocated
   * @param device The flag whether device data will be allocated
   */
  template <bool initialize>
  void createInternal(const std::vector<index_type> & n, bool host, bool device);
  /**
   * The internal method to perform a memory copy
   * @tparam TargetSpace The Kokkos memory space of target data
   * @tparam Sourcespace The Kokkos memory space of source data
   * @param target The pointer to the target data
   * @param source The pointer to the source data
   * @param n The number of entries to copy
   */
  template <typename TargetSpace, typename SourceSpace>
  void copyInternal(T * target, const T * source, index_type n);
#endif

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Allocate host data for an initialized array that has not allocated host data
   * @tparam initialize Whether to initialize host data (calls default constructor)
   */
  template <bool initialize>
  void allocHost();
  /**
   * Allocate device data for an initialized array that has not allocated device data
   */
  void allocDevice();
#endif
  /**
   * Free host data
   */
  void freeHost();
  /**
   * Free device data
   */
  void freeDevice();

  /**
   * Reference counter
   */
  std::shared_ptr<unsigned int> _counter;
  /**
   * Flag whether array was initialized
   */
  bool _is_init = false;
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
  index_type _size = 0;
  /**
   * Memory layout type
   */
  const LayoutType _layout;
};

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::freeHost()
{
  if (!_is_host_alloc)
    return;

  if (_is_host_alias)
  {
    _host_data = nullptr;
    _is_host_alias = false;
  }
  else
  {
    if (!_is_malloc)
      // Allocated by new
      delete[] _host_data;
    else
    {
      // Allocated by malloc
      for (index_type i = 0; i < _size; ++i)
        _host_data[i].~T();

      std::free(_host_data);
    }
  }

  _is_host_alloc = false;
  _is_malloc = false;
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::freeDevice()
{
  if (!_is_device_alloc)
    return;

  if (_is_device_alias)
  {
    _device_data = nullptr;
    _is_device_alias = false;
  }
  else
    Moose::Kokkos::free(_device_data);

  _is_device_alloc = false;
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::destroy()
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
    freeHost();
    freeDevice();
  }

  _size = 0;

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _n[i] = 0;
    _s[i] = 0;
    _d[i] = 0;
  }

  _is_init = false;
  _is_offset = false;
  _is_malloc = false;
  _is_host_alloc = false;
  _is_device_alloc = false;
  _is_host_alias = false;
  _is_device_alias = false;

  _counter.reset();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::shallowCopy(const ArrayBase<T, dimension, index_type> & array)
{
  if (_layout != array._layout)
    mooseError("Kokkos array error: cannot shallow copy arrays with different layouts.");

  destroy();

  _counter = array._counter;

  _size = array._size;

  for (unsigned int i = 0; i < dimension; ++i)
  {
    _n[i] = array._n[i];
    _s[i] = array._s[i];
    _d[i] = array._d[i];
  }

  _is_init = array._is_init;
  _is_offset = array._is_offset;
  _is_malloc = array._is_malloc;
  _is_host_alloc = array._is_host_alloc;
  _is_device_alloc = array._is_device_alloc;
  _is_host_alias = array._is_host_alias;
  _is_device_alias = array._is_device_alias;

  _host_data = array._host_data;
  _device_data = array._device_data;
}

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::aliasHost(T * ptr)
{
  if (!_is_init)
    mooseError("Kokkos array error: attempted to alias host data before array initialization.");

  if (_is_host_alloc && !_is_host_alias)
    mooseError("Kokkos array error: cannot alias host data because host data was not aliased.");

  _host_data = ptr;
  _is_host_alloc = true;
  _is_host_alias = true;
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::aliasDevice(T * ptr)
{
  if (!_is_init)
    mooseError("Kokkos array error: attempted to alias device data before array initialization.");

  if (_is_device_alloc && !_is_device_alias)
    mooseError("Kokkos array error: cannot alias device data because device data was not aliased.");

  _device_data = ptr;
  _is_device_alloc = true;
  _is_device_alias = true;
}

template <typename T, unsigned int dimension, typename index_type>
template <bool initialize>
void
ArrayBase<T, dimension, index_type>::allocHost()
{
  if (_is_host_alloc)
    return;

  if constexpr (initialize)
  {
    static_assert(
        std::is_default_constructible<T>::value,
        "Data type is not default-constructible. Initialization argument should be set to false.");

    _host_data = new T[_size];
  }
  else
    _host_data = static_cast<T *>(std::malloc(_size * sizeof(T)));

  _is_host_alloc = true;
  _is_malloc = !initialize;
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::allocDevice()
{
  if (_is_device_alloc)
    return;

  _device_data =
      static_cast<T *>(::Kokkos::kokkos_malloc<ExecSpace::memory_space>(_size * sizeof(T)));

  _is_device_alloc = true;
}

template <typename T, unsigned int dimension, typename index_type>
template <bool host, bool device, bool initialize>
void
ArrayBase<T, dimension, index_type>::createInternal(const std::vector<index_type> & n)
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

  uint64_t overflow_checker = 1;

  _size = 1;
  _s[0] = 1;

  for (const auto i : make_range(dimension))
  {
    overflow_checker *= n[i];

    _n[i] = n[i];
    _size *= n[i];
  }

  if (overflow_checker > std::numeric_limits<index_type>::max())
    mooseError("Kokkos array error: the dimensions provided (",
               Moose::stringify(n),
               ") has the total size of ",
               overflow_checker,
               " which exceeds the limit of ",
               MooseUtils::prettyCppType<index_type>(),
               ".");

  if (_layout == LayoutType::LEFT)
  {
    _s[0] = 1;

    for (unsigned int i = 1; i < dimension; ++i)
      _s[i] = _s[i - 1] * _n[i - 1];
  }
  else
  {
    _s[dimension - 1] = 1;

    for (int i = dimension - 2; i >= 0; --i)
      _s[i] = _s[i + 1] * _n[i + 1];
  }

  if constexpr (host)
    allocHost<initialize>();

  if constexpr (device)
    allocDevice();

  _is_init = true;
}

template <typename T, unsigned int dimension, typename index_type>
template <bool initialize>
void
ArrayBase<T, dimension, index_type>::createInternal(const std::vector<index_type> & n,
                                                    bool host,
                                                    bool device)
{
  if (host && device)
    createInternal<true, true, initialize>(n);
  else if (host && !device)
    createInternal<true, false, initialize>(n);
  else if (!host && device)
    createInternal<false, true, initialize>(n);
  else
    createInternal<false, false, initialize>(n);
}

template <typename T, unsigned int dimension, typename index_type>
template <bool host, bool device, bool initialize, typename... size_type>
void
ArrayBase<T, dimension, index_type>::createInternal(size_type... n)
{
  static_assert((std::is_convertible<size_type, index_type>::value && ...),
                "All arguments must be convertible to index_type");
  static_assert(sizeof...(n) == dimension, "Number of arguments should match array dimension");

  std::vector<index_type> dims;
  (dims.push_back(n), ...);

  createInternal<host, device, initialize>(dims);
}

template <typename T, unsigned int dimension, typename index_type>
template <typename TargetSpace, typename SourceSpace>
void
ArrayBase<T, dimension, index_type>::copyInternal(T * target, const T * source, index_type n)
{
  ::Kokkos::Impl::DeepCopy<TargetSpace, SourceSpace>(target, source, n * sizeof(T));
  ::Kokkos::fence();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::offset(const std::vector<signed_index_type> & d)
{
  if (d.size() > dimension)
    mooseError("Kokkos array error: the number of offsets provided (",
               d.size(),
               ") cannot be larger than the array dimension (",
               dimension,
               ").");

  for (const auto i : index_range(d))
    _d[i] = d[i];

  _is_offset = true;
}

template <typename T, unsigned int dimension, typename index_type>
template <typename... offset_type>
void
ArrayBase<T, dimension, index_type>::offset(offset_type... d)
{
  static_assert((std::is_convertible<offset_type, signed_index_type>::value && ...),
                "All arguments must be convertible to signed_index_type");
  static_assert(sizeof...(d) == dimension, "Number of arguments should match array dimension");

  std::vector<signed_index_type> offsets;
  (offsets.push_back(d), ...);

  offset(offsets);
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::copyToDevice()
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
      mooseError("Kokkos array error: cannot copy from host to device because device memory "
                 "was not allocated. Cannot allocate device memory for copy because the array is "
                 "being shared.");
  }

  // Copy from host to device
  copyInternal<MemSpace, ::Kokkos::HostSpace>(_device_data, _host_data, _size);
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::copyToHost()
{
  // If device side memory is not allocated, do nothing
  if (!_is_device_alloc)
    return;

  // If host side memory is not allocated,
  if (!_is_host_alloc)
  {
    if (_counter.use_count() == 1)
      // allocate memory if this array is not shared with other arrays
      allocHost<false>();
    else
      // print error if this array is shared with other arrays
      mooseError("Kokkos array error: cannot copy from device to host because host memory "
                 "was not allocated. Cannot allocate host memory for copy because the array is "
                 "being shared.");
  }

  // Copy from device to host
  copyInternal<::Kokkos::HostSpace, MemSpace>(_host_data, _device_data, _size);
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::moveToDevice(bool should_free_host)
{
  static_assert(!is_kokkos_array<T>::value,
                "moveToDevice() not allowed for a nested array whose data type is another array.");

  if (should_free_host && _counter.use_count() > 1)
    mooseError("Kokkos array error: cannot move array from host to device because there is at "
               "least one shallow copy of this array still alive.");

  copyToDevice();

  if (_counter.use_count() == 1)
    freeHost();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::moveToHost(bool should_free_device)
{
  if (should_free_device && _counter.use_count() > 1)
    mooseError("Kokkos array error: cannot move array from device to host because there is at "
               "least one shallow copy of this array still alive.");

  copyToHost();

  if (_counter.use_count() == 1)
    freeDevice();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::copyIn(const T * ptr,
                                            MemcpyType dir,
                                            index_type n,
                                            index_type offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copy in data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyType::HOST_TO_HOST)
  {
    // If host side memory is not allocated, print error
    if (!_is_host_alloc)
      mooseError(
          "Kokkos array error: cannot copy in to the array because host memory was not allocated.");

    // Copy from host to host
    copyInternal<::Kokkos::HostSpace, ::Kokkos::HostSpace>(_host_data + offset, ptr, n);
  }
  else if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    // If device side memory is not allocated, print error
    if (!_is_device_alloc)
      mooseError("Kokkos array error: cannot copy in to the array because device memory was not "
                 "allocated.");

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(_device_data + offset, ptr, n);
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    // If host side memory is not allocated, print error
    if (!_is_host_alloc)
      mooseError(
          "Kokkos array error: cannot copy in to the array because host memory was not allocated.");

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(_host_data + offset, ptr, n);
  }
  else if (dir == MemcpyType::DEVICE_TO_DEVICE)
  {
    // If device side memory is not allocated, print error
    if (!_is_device_alloc)
      mooseError("Kokkos array error: cannot copy in to the array because device memory was not "
                 "allocated.");

    // Copy from device to device
    copyInternal<MemSpace, MemSpace>(_device_data + offset, ptr, n);
  }
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::copyOut(T * ptr,
                                             MemcpyType dir,
                                             index_type n,
                                             index_type offset)
{
  if (n > _size)
    mooseError("Kokkos array error: cannot copy out data larger than the array size.");

  if (offset > _size)
    mooseError("Kokkos array error: offset cannot be larger than the array size.");

  if (dir == MemcpyType::HOST_TO_HOST)
  {
    // If host side memory is not allocated, print error
    if (!_is_host_alloc)
      mooseError("Kokkos array error: cannot copy out from the array because host memory was not "
                 "allocated.");

    // Copy from host to host
    copyInternal<::Kokkos::HostSpace, ::Kokkos::HostSpace>(ptr, _host_data + offset, n);
  }
  else if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    // If host side memory is not allocated, print error
    if (!_is_host_alloc)
      mooseError("Kokkos array error: cannot copy out from the array because host memory was not "
                 "allocated.");

    // Copy from host to device
    copyInternal<MemSpace, ::Kokkos::HostSpace>(ptr, _host_data + offset, n);
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    // If device side memory is not allocated, print error
    if (!_is_device_alloc)
      mooseError("Kokkos array error: cannot copy out from the array because device memory was not "
                 "allocated.");

    // Copy from device to host
    copyInternal<::Kokkos::HostSpace, MemSpace>(ptr, _device_data + offset, n);
  }
  else if (dir == MemcpyType::DEVICE_TO_DEVICE)
  {
    // If device side memory is not allocated, print error
    if (!_is_device_alloc)
      mooseError("Kokkos array error: cannot copy out from the array because device memory was not "
                 "allocated.");

    // Copy from device to device
    copyInternal<MemSpace, MemSpace>(ptr, _device_data + offset, n);
  }
}

template <typename T>
void
copyToDeviceInner(T & /* data */)
{
}

template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
void
copyToDeviceInner(Array<T, dimension, index_type, layout> & data)
{
  data.copyToDeviceNested();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::copyToDeviceNested()
{
  for (index_type i = 0; i < _size; ++i)
    copyToDeviceInner(_host_data[i]);

  copyToDevice();
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::deepCopy(const ArrayBase<T, dimension, index_type> & array)
{
  if (_layout != array._layout)
    mooseError("Kokkos array error: cannot deep copy arrays with different layouts.");

  if (ArrayDeepCopy<T>::value && !array._is_host_alloc)
    mooseError(
        "Kokkos array error: cannot deep copy using constructor from array without host data.");

  std::vector<index_type> n(std::begin(array._n), std::end(array._n));

  createInternal<false>(n, array._is_host_alloc, array._is_device_alloc);

  if constexpr (ArrayDeepCopy<T>::value)
  {
    for (index_type i = 0; i < _size; ++i)
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

  _is_offset = array._is_offset;
}

template <typename T, unsigned int dimension, typename index_type>
void
ArrayBase<T, dimension, index_type>::swap(ArrayBase<T, dimension, index_type> & array)
{
  ArrayBase<T, dimension, index_type> clone(_layout);

  clone.shallowCopy(*this);
  this->shallowCopy(array);
  array.shallowCopy(clone);
}

template <typename T, unsigned int dimension, typename index_type>
auto &
ArrayBase<T, dimension, index_type>::operator=(const T & scalar)
{
  if (_is_host_alloc)
    std::fill_n(_host_data, _size, scalar);

  if (_is_device_alloc)
    ::Kokkos::Experimental::fill_n(ExecSpace(), deviceView(), _size, scalar);

  return *this;
}

template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
void
dataStore(std::ostream & stream, Array<T, dimension, index_type, layout> & array, void * context)
{
  using ::dataStore;

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

    array.copyOut(data, MemcpyType::DEVICE_TO_HOST, array.size());

    for (index_type i = 0; i < array.size(); ++i)
      dataStore(stream, data[i], context);

    std::free(data);
  }
  else
    for (auto & value : array)
      dataStore(stream, value, context);
}

template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
void
dataLoad(std::istream & stream, Array<T, dimension, index_type, layout> & array, void * context)
{
  using ::dataLoad;

  bool is_alloc;
  dataLoad(stream, is_alloc, nullptr);

  if (!is_alloc)
    return;

  std::string from_type_name;
  dataLoad(stream, from_type_name, nullptr);

  if (from_type_name != typeid(T).name())
    mooseError("Kokkos array error: cannot load array because the stored array is of type '",
               MooseUtils::prettyCppType(libMesh::demangle(from_type_name.c_str())),
               "' but the loading array is of type '",
               MooseUtils::prettyCppType(libMesh::demangle(typeid(T).name())),
               "'.");

  unsigned int from_dimension;
  dataLoad(stream, from_dimension, nullptr);

  if (from_dimension != dimension)
    mooseError("Kokkos array error: cannot load array because the stored array is ",
               from_dimension,
               "D but the loading array is ",
               dimension,
               "D.");

  std::vector<index_type> from_n(dimension);
  std::vector<index_type> n(dimension);

  for (unsigned int dim = 0; dim < dimension; ++dim)
  {
    dataLoad(stream, from_n[dim], nullptr);
    n[dim] = array.n(dim);
  }

  if (from_n != n)
    mooseError("Kokkos array error: cannot load array because the stored array has dimensions (",
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

    array.copyIn(data.data(), MemcpyType::HOST_TO_DEVICE, array.size());
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
 * Kokkos object if it is used on device, because the reference refers to a host object and
 * therefore is not accessible on device. If storing it as a reference is required, see
 * ReferenceWrapper.
 * @tparam T The data type
 * @tparam dimension The array dimension size
 * @tparam index_type The array index type
 * @tparam layout The memory layout type
 */
///@{
template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
class Array : public ArrayBase<T, dimension, index_type>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, dimension, index_type);
#endif

public:
  /**
   * Default constructor
   */
  Array() : ArrayBase<T, dimension, index_type>(layout) {}
  /**
   * Copy constructor
   */
  Array(const Array<T, dimension, index_type, layout> & array)
    : ArrayBase<T, dimension, index_type>(array)
  {
  }
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given dimensions
   * This allocates both host and device data
   * @param n The size of each dimension
   */
  template <typename... size_type>
  Array(size_type... n) : ArrayBase<T, dimension, index_type>(layout, n...)
  {
  }
#endif

  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, dimension, index_type, layout> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get an array entry
   * @param i The index of each dimension
   * @returns The reference of the entry depending on the architecture this function is being called
   * on
   */
  template <typename... indices>
  KOKKOS_FUNCTION T & operator()(indices... i) const;
  /**
   * Get an array entry using indices stored in an array
   * @param idx The array storing the indices
   * @returns The reference of the entry depending on the architecture this function is being
   * called on
   */
  KOKKOS_FUNCTION T & operator()(const signed_index_type (&idx)[dimension]) const
  {
    return operatorInternal(idx, std::make_integer_sequence<unsigned int, dimension>{});
  }
#endif

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Internal method for calling operator() with array indices
   */
  template <unsigned int... i>
  KOKKOS_FUNCTION T & operatorInternal(const signed_index_type (&idx)[dimension],
                                       std::integer_sequence<unsigned int, i...>) const
  {
    return operator()(idx[i]...);
  }
#endif
};

#ifdef MOOSE_KOKKOS_SCOPE
template <typename T, unsigned int dimension, typename index_type, LayoutType layout>
template <typename... indices>
KOKKOS_FUNCTION T &
Array<T, dimension, index_type, layout>::operator()(indices... i) const
{
  static_assert((std::is_convertible<indices, signed_index_type>::value && ...),
                "All arguments must be convertible to signed_index_type");
  static_assert(sizeof...(i) == dimension, "Number of arguments should match array dimension");

#ifndef NDEBUG
  {
    signed_index_type idx[dimension] = {static_cast<signed_index_type>(i)...};

    for (unsigned int d = 0; d < sizeof...(i); ++d)
      KOKKOS_ASSERT(idx[d] - _d[d] >= 0 && static_cast<index_type>(idx[d] - _d[d]) < _n[d]);
  }
#endif

  index_type idx = 0;
  unsigned int d = 0;

  if (_is_offset)
  {
    if constexpr (layout == LayoutType::LEFT)
      (((idx += (d == 0 ? static_cast<signed_index_type>(i) - _d[d]
                        : (static_cast<signed_index_type>(i) - _d[d]) * _s[d])),
        ++d),
       ...);
    else
      (((idx += (d == dimension - 1 ? static_cast<signed_index_type>(i) - _d[d]
                                    : (static_cast<signed_index_type>(i) - _d[d]) * _s[d])),
        ++d),
       ...);
  }
  else
  {
    if constexpr (layout == LayoutType::LEFT)
      (((idx +=
         (d == 0 ? static_cast<signed_index_type>(i) : static_cast<signed_index_type>(i) * _s[d])),
        ++d),
       ...);
    else
      (((idx += (d == dimension - 1 ? static_cast<signed_index_type>(i)
                                    : static_cast<signed_index_type>(i) * _s[d])),
        ++d),
       ...);
  }

  return this->operator[](idx);
}
#endif

template <typename T, typename index_type>
class Array<T, 1, index_type, LayoutType::LEFT> : public ArrayBase<T, 1, index_type>
{
#ifdef MOOSE_KOKKOS_SCOPE
  usingKokkosArrayBaseMembers(T, 1, index_type);
#endif

public:
  /**
   * Default constructor
   */
  Array() : ArrayBase<T, 1, index_type>(LayoutType::LEFT) {}
  /**
   * Copy constructor
   */
  Array(const Array<T, 1, index_type, LayoutType::LEFT> & array)
    : ArrayBase<T, 1, index_type>(array)
  {
  }
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Constructor
   * Initialize and allocate array with given size
   * This allocates both host and device data
   * @param n The array size
   */
  Array(index_type n) : ArrayBase<T, 1, index_type>(LayoutType::LEFT, n) {}
  /**
   * Constructor
   * Initialize and allocate array by copying a standard vector variable
   * This allocates and copies to both host and device data
   * @param vector The standard vector variable to copy
   */
  Array(const std::vector<T> & vector) : ArrayBase<T, 1, index_type>(LayoutType::LEFT)
  {
    *this = vector;
  }
#endif

  /**
   * Shallow copy another Kokkos array
   * @param array The Kokkos array to be shallow copied
   */
  auto & operator=(const Array<T, 1, index_type, LayoutType::LEFT> & array)
  {
    this->shallowCopy(array);

    return *this;
  }

#ifdef MOOSE_KOKKOS_SCOPE
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
    this->template createInternal<host, device, false>({static_cast<index_type>(vector.size())});

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
   * @param i The array index
   * @returns The reference of the entry depending on the architecture this function is being
   * called on
   */
  KOKKOS_FUNCTION T & operator()(signed_index_type i) const
  {
    KOKKOS_ASSERT(i - _d[0] >= 0 && static_cast<index_type>(i - _d[0]) < _n[0]);

    if (_is_offset)
      return this->operator[](i - _d[0]);
    else
      return this->operator[](i);
  }
  /**
   * Device BLAS operations
   */
  ///@{
  /**
   * Perform \p a * \p x \p op \p b * \p y and write the result to this array
   * @param accumulate Whether to accumulate or overwrite the result
   */
  void axby(const T a,
            const Array<T, 1, index_type, LayoutType::LEFT> & x,
            const char op,
            const T b,
            const Array<T, 1, index_type, LayoutType::LEFT> & y,
            const bool accumulate = false);
  /**
   * Scale \p x with \p a and write the result to this array
   */
  void scal(const T a, const Array<T, 1, index_type, LayoutType::LEFT> & x);
  /**
   * Scale this array with \p a
   */
  void scal(const T a);
  /**
   * Perform dot product between this array and \p x
   */
  T dot(const Array<T, 1, index_type, LayoutType::LEFT> & x);
  /**
   * Compute 2-norm of this array
   */
  T nrm2();
  ///}@
#endif
};
///@}

template <typename T, typename index_type = dof_id_type>
using Array1D = Array<T, 1, index_type, LayoutType::LEFT>;
template <typename T, typename index_type = dof_id_type, LayoutType layout = LayoutType::LEFT>
using Array2D = Array<T, 2, index_type, layout>;
template <typename T, typename index_type = dof_id_type, LayoutType layout = LayoutType::LEFT>
using Array3D = Array<T, 3, index_type, layout>;
template <typename T, typename index_type = dof_id_type, LayoutType layout = LayoutType::LEFT>
using Array4D = Array<T, 4, index_type, layout>;
template <typename T, typename index_type = dof_id_type, LayoutType layout = LayoutType::LEFT>
using Array5D = Array<T, 5, index_type, layout>;

} // namespace Moose::Kokkos
