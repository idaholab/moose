//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"

namespace RayTracingPackingUtils
{

/**
 * Like std::copy, but passes the input iterator by reference
 */
template <class Cont, class InputIt>
void unpackCopy(Cont & container, InputIt & input_iterator);

/**
 * Packs the data in \p values into the iterator \p out and minimizes memory
 * storage in said iterator.
 *
 * In specific, ValueType is packed into BufferType at a byte level. That is,
 * if sizeof(ValueType) == 4 and sizeof(BufferType) == 8, two values of type ValueType
 * objects will be stored in a single value of type BufferType.
 */
template <typename BufferType, typename BufferIter, typename ValueType>
void reinterpretPackCopy(const std::vector<ValueType> & values, BufferIter & out);

/**
 * Packs the data from \p in into the vector \p values.
 *
 * \p values MUST be resized ahead of time in order to know how much to advance \p in.
 *
 * This is to be used in the unpacking of values stored by reinterpretPackCopy().
 */
template <typename BufferType, typename BufferIter, typename ValueType>
void reinterpretUnpackCopy(std::vector<ValueType> & values, BufferIter & in);

/**
 * Gets the minimum number of values of BufferType needed to represent
 * \p input_size values of ValueType
 *
 * To be used with sizing for reinterpretPackCopy() and reinterpretUnpackCopy().
 */
template <typename ValueType, typename BufferType>
std::size_t reinterpretCopySize(const std::size_t input_size);

/**
 * Unpacks the mixed-values from \p in into \p values that were packed with mixedPack().
 */
template <typename BufferType, typename BufferIter, typename... ValueTypes>
void mixedUnpack(BufferIter & in, ValueTypes &... values);

/**
 * Packs the mixed-type values in \p values into \p out to be unpacked with mixedUnpack().
 *
 * Uses as few entries in \p out needed to represent \p values in order.
 */
template <typename BufferType, typename BufferIter, typename... ValueTypes>
void mixedPack(BufferIter & out, ValueTypes const &... values);

/**
 * Gets the number of BufferType required to store the expanded InputTypes for use with
 * mixedPack() and mixedUnpack().
 *
 * Can be stored as constexpr to evaluate the size at compile time only.
 */
template <typename BufferType, typename... InputTypes>
constexpr std::size_t mixedPackSize();

/**
 * Packs \p value into a value of type BufferType at a byte level, to be unpacked with
 * the unpack() routines in this namespace.
 */
template <typename BufferType, typename ValueType>
BufferType pack(const ValueType value);

/**
 * Unpacks \p value_as_buffer_type (which is packed with pack()) into \p value at a byte level.
 */
template <typename BufferType, typename ValueType>
void unpack(const BufferType value_as_buffer_type, ValueType & value);

/**
 * Packs the ID of \p elem into a type of BufferType to be unpacked later into another const Elem *
 * with unpack().
 */
template <typename BufferType>
BufferType pack(const Elem * elem, MeshBase * mesh_base = nullptr);

/**
 * Unpacks the const Elem * from \p id_as_buffer_type (packed using pack()) into \p elem.
 */
template <typename BufferType>
void unpack(const Elem *& elem, const BufferType id_as_buffer_type, MeshBase * mesh_base);

namespace detail
{

/**
 * Helper for mixedPackSize().
 *
 * Called after evaluating the last InputType, and increases the size
 * if any offset remains
 */
template <typename BufferType>
constexpr std::size_t
mixedPackSizeHelper(const std::size_t offset, const std::size_t size)
{
  return offset ? size + 1 : size;
}

/**
 * Recursive helper for mixedPackSize().
 *
 * Recurses through the types (in InputType and rest), and increments the
 * offset and size as needed
 */
template <typename BufferType, typename InputType, typename... Rest>
constexpr std::size_t
mixedPackSizeHelper(std::size_t offset, std::size_t size)
{
  return offset + sizeof(InputType) > sizeof(BufferType)
             ? mixedPackSizeHelper<BufferType, Rest...>(sizeof(InputType), ++size)
             : mixedPackSizeHelper<BufferType, Rest...>(offset + sizeof(InputType), size);
}

/**
 * Helper for mixedUnpack()
 */
template <typename BufferType, typename BufferIter, typename ValueType>
void
mixedUnpackHelper(BufferIter & in,
                  const BufferType *& src,
                  std::size_t & src_offset,
                  ValueType & output)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "ValueType will not fit into BufferType");

  if (src_offset + sizeof(ValueType) > sizeof(BufferType))
  {
    src = &(*in++);
    src_offset = 0;
  }

  std::memcpy(&output, (char *)src + src_offset, sizeof(ValueType));
  src_offset += sizeof(ValueType);
}

/**
 * Helper for mixedPack()
 */
template <typename BufferIter, typename BufferType, typename ValueType>
void
mixedPackHelper(BufferIter & out,
                BufferType & dest,
                std::size_t & dest_offset,
                const ValueType & input)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "ValueType will not fit into BufferType");

  if (dest_offset + sizeof(ValueType) > sizeof(BufferType))
  {
    out++ = dest;
    dest_offset = 0;
  }

  std::memcpy((char *)&dest + dest_offset, &input, sizeof(ValueType));
  dest_offset += sizeof(ValueType);
}
}

template <class Cont, class InputIt>
void
unpackCopy(Cont & container, InputIt & input_iterator)
{
  auto first = container.begin();
  while (first != container.end())
    *first++ = *input_iterator++;
}

template <typename BufferType, typename BufferIter, typename ValueType>
void
reinterpretPackCopy(const std::vector<ValueType> & values, BufferIter & out)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "ValueType will not fit into BufferType");

  BufferType dest;
  const ValueType * src = values.data();

  std::size_t dest_offset = 0;
  for (std::size_t i = 0; i < values.size(); ++i)
  {
    if (dest_offset + sizeof(ValueType) > sizeof(BufferType))
    {
      out++ = dest;
      dest_offset = 0;
    }

    std::memcpy((char *)&dest + dest_offset, &src[i], sizeof(ValueType));
    dest_offset += sizeof(ValueType);
  }

  if (dest_offset)
    out++ = dest;
}

template <typename BufferType, typename BufferIter, typename ValueType>
void
reinterpretUnpackCopy(std::vector<ValueType> & values, BufferIter & in)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "ValueType will not fit into BufferType");

  ValueType * dest = values.data();
  const BufferType * src = nullptr;

  std::size_t src_offset = sizeof(BufferType);
  for (std::size_t i = 0; i < values.size(); ++i)
  {
    if (src_offset + sizeof(ValueType) > sizeof(BufferType))
    {
      src = &(*in++);
      src_offset = 0;
    }

    std::memcpy(&dest[i], (char *)src + src_offset, sizeof(ValueType));
    src_offset += sizeof(ValueType);
  }
}

template <typename ValueType, typename BufferType>
std::size_t
reinterpretCopySize(const std::size_t input_size)
{
  const double input_per_output = std::floor(sizeof(BufferType) / sizeof(ValueType));
  return (std::size_t)std::ceil((double)input_size / input_per_output);
}

template <typename BufferType, typename BufferIter, typename... ValueTypes>
void
mixedUnpack(BufferIter & in, ValueTypes &... values)
{
  std::size_t src_offset = sizeof(BufferType);
  const BufferType * src = nullptr;

  int expander[] = {
      0,
      ((void)detail::mixedUnpackHelper(in, src, src_offset, std::forward<ValueTypes &>(values)),
       0)...};
  (void)expander;
}

template <typename BufferType, typename BufferIter, typename... ValueTypes>
void
mixedPack(BufferIter & out, ValueTypes const &... values)
{
  std::size_t dest_offset = 0;
  BufferType dest;

  int expander[] = {0,
                    ((void)detail::mixedPackHelper(
                         out, dest, dest_offset, std::forward<ValueTypes const &>(values)),
                     0)...};
  (void)expander;

  if (dest_offset)
    out++ = dest;
}

template <typename BufferType, typename... InputTypes>
constexpr std::size_t
mixedPackSize()
{
  // Call the recursive helper with an initial offset and size of 0
  return detail::mixedPackSizeHelper<BufferType, InputTypes...>(/* offset = */ 0, /* size = */ 0);
}

template <typename BufferType, typename ValueType>
BufferType
pack(const ValueType value)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "Value will won't fit into buffer type");

  BufferType value_as_buffer_type;
  std::memcpy(&value_as_buffer_type, &value, sizeof(ValueType));
  return value_as_buffer_type;
}

template <typename BufferType, typename ValueType>
void
unpack(const BufferType value_as_buffer_type, ValueType & value)
{
  static_assert(sizeof(ValueType) <= sizeof(BufferType), "Value will won't fit into buffer type");
  std::memcpy(&value, &value_as_buffer_type, sizeof(ValueType));
}

template <typename BufferType>
BufferType
pack(const Elem * elem, MeshBase * libmesh_dbg_var(mesh_base /* = nullptr */))
{
  const dof_id_type id = elem ? elem->id() : DofObject::invalid_id;
  mooseAssert(mesh_base ? mesh_base->query_elem_ptr(id) == elem : true,
              "Elem doesn't exist in mesh");

  return pack<BufferType>(id);
}

template <typename BufferType>
void
unpack(const Elem *& elem, const BufferType id_as_buffer_type, MeshBase * mesh_base)
{
  dof_id_type id;
  unpack<BufferType>(id_as_buffer_type, id);

  elem = (id == DofObject::invalid_id ? nullptr : mesh_base->query_elem_ptr(id));
}
}
