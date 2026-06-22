//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ADReal.h"
#include "MooseTypes.h"
#include "HashMap.h"
#include "MooseError.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"
#include "ColumnMajorMatrix.h"
#include "UniqueStorage.h"
#include "TwoVector.h"

#include "libmesh/parallel.h"
#include "libmesh/parameters.h"
#include "libmesh/numeric_vector.h"

#ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
#include <type_traits>
#endif

// C++ includes
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

#include "mtwist.h"

namespace libMesh
{
template <typename T>
class DenseMatrix;
template <typename T>
class DenseVector;
template <typename T>
class VectorValue;
template <typename T>
class TensorValue;
class Elem;
class FEType;
class Point;
}

template <typename T>
inline void dataStore(std::ostream & stream, T & v, void * /*context*/);

// DO NOT MODIFY THE NEXT LINE - It is used by MOOSEDocs
// *************** Global Store Declarations *****************
template <typename T>
inline void
dataStore(std::ostream &, T &, void * /*context*/)
{
  static_assert(sizeof(T) == 0, "You must write a dataStore specialization for your type");
}

namespace moose
{
namespace internal
{
template <typename T>
void
dataStoreSizeof(std::ostream & stream, T & v)
{
  stream.write((char *)&v, sizeof(v));
  mooseAssert(!stream.bad(), "Failed to store");
}
}
}

// clang-format off
#define dataStoreSizeofSpecialization(Type) \
template <> \
inline void \
dataStore(std::ostream & stream, Type & v, void *) \
{ \
  moose::internal::dataStoreSizeof(stream, v); \
} \
template <> \
inline void \
dataStore(std::ostream & stream, const Type & v, void *) \
{ \
  moose::internal::dataStoreSizeof(stream, v); \
}

dataStoreSizeofSpecialization(bool)
dataStoreSizeofSpecialization(char)
dataStoreSizeofSpecialization(short int)
dataStoreSizeofSpecialization(int)
dataStoreSizeofSpecialization(long int)
dataStoreSizeofSpecialization(long long int)
dataStoreSizeofSpecialization(unsigned char)
dataStoreSizeofSpecialization(short unsigned int)
dataStoreSizeofSpecialization(unsigned int)
dataStoreSizeofSpecialization(long unsigned int)
dataStoreSizeofSpecialization(long long unsigned int)
dataStoreSizeofSpecialization(float)
dataStoreSizeofSpecialization(double)
dataStoreSizeofSpecialization(long double)

#define dataStoreEnum(EnumType, IntType) \
template <> \
inline void dataStore(std::ostream & stream, EnumType & enum_type, void * ctx) \
{ \
  auto stored = static_cast<IntType>(enum_type); \
  dataStore(stream, stored, ctx); \
}

dataStoreEnum(libMesh::FEFamily, int)
dataStoreEnum(libMesh::ParallelType, int)
dataStoreEnum(Moose::CoordinateSystemType, int)

template <>
void dataStore(std::ostream &, mt_state &, void *);
// clang-format on
template <>
void dataStore(std::ostream & stream, libMesh::Point & p, void * context);
template <>
void dataStore(std::ostream & stream, const libMesh::Point & p, void * context);
// Specializations (defined in .C)
template <>
void dataStore(std::ostream & stream, std::string & v, void * context);
template <>
void dataStore(std::ostream & stream, VariableName & v, void * context);
template <>
void dataStore(std::ostream & stream, UserObjectName & v, void * context);
template <>
void dataStore(std::ostream & stream, libMesh::FEType & v, void * context);
template <>
void dataStore(std::ostream & stream, const Elem *& e, void * context);
template <>
void dataStore(std::ostream & stream, const Node *& n, void * context);
template <>
void dataStore(std::ostream & stream, Elem *& e, void * context);
template <>
void dataStore(std::ostream & stream, Node *& n, void * context);
template <>
void dataStore(std::ostream & stream, std::stringstream & s, void * context);
template <>
void dataStore(std::ostream & stream, ADReal & dn, void * context);
template <>
void dataStore(std::ostream & stream, libMesh::Parameters & p, void * context);

template <>
/**
 * Stores an owned numeric vector.
 *
 * This should be used in lieu of the NumericVector<Number> & implementation
 * when the vector may not necessarily be initialized yet on the loading of
 * the data. It stores the partitioning (total and local number of entries).
 *
 * Requirements: the unique_ptr must exist (cannot be null), the vector
 * cannot be ghosted, and the provided context must be the Communicator.
 */
void dataStore(std::ostream & stream,
               std::unique_ptr<libMesh::NumericVector<libMesh::Number>> & v,
               void * context);

template <typename T>
void dataStore(std::ostream & stream, std::shared_ptr<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::unique_ptr<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, libMesh::TensorValue<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, libMesh::DenseMatrix<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, libMesh::DenseVector<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, libMesh::VectorValue<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::vector<T> & v, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::set<T> & s, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::list<T> & l, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::deque<T> & l, void * context);

template <typename T, typename U>
void dataStore(std::ostream & stream, std::map<T, U> & m, void * context);

template <typename T, typename U>
void dataStore(std::ostream & stream, std::unordered_map<T, U> & m, void * context);

template <typename T>
void dataStore(std::ostream & stream, std::unordered_set<T> & s, void * context);

template <typename T, std::size_t N>
void dataStore(std::ostream & stream, std::array<T, N> & arr, void * context);

template <typename T, int Rows, int Cols>
void dataStore(std::ostream & stream, Eigen::Matrix<T, Rows, Cols> & v, void * context);

template <typename T>
inline void
dataStore(std::ostream & /*stream*/, T *& /*v*/, void * /*context*/)
{
  mooseError("Attempting to store a raw pointer type: \"",
             libMesh::demangle(typeid(T).name()),
             " *\" as restartable data!\nWrite a custom dataStore() template specialization!\n\n");
}
// clang-format on

template <typename T, typename U>
inline void
dataStore(std::ostream & stream, std::pair<T, U> & p, void * context)
{
  dataStore(stream, p.first, context);
  dataStore(stream, p.second, context);
}

inline void
dataStore(std::ostream & ostream, std::vector<bool> & v, void * ctx)
{
  for (const bool val : v)
    dataStore(ostream, val, ctx);
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::vector<T> & v, void * context)
{
  // First store the size of the vector
  unsigned int size = v.size();
  dataStore(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
    dataStore(stream, v[i], context);
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::shared_ptr<T> & v, void * context)
{
  T * tmp = v.get();

  dataStore(stream, tmp, context);
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::unique_ptr<T> & v, void * context)
{
  T * tmp = v.get();

  dataStore(stream, tmp, context);
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::set<T> & s, void * context)
{
  // First store the size of the set
  unsigned int size = s.size();
  dataStore(stream, size, nullptr);

  typename std::set<T>::iterator it = s.begin();
  typename std::set<T>::iterator end = s.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    dataStore(stream, x, context);
  }
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::list<T> & l, void * context)
{
  // First store the size of the set
  unsigned int size = l.size();
  dataStore(stream, size, nullptr);

  typename std::list<T>::iterator it = l.begin();
  typename std::list<T>::iterator end = l.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    dataStore(stream, x, context);
  }
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::deque<T> & l, void * context)
{
  // First store the size of the container
  unsigned int size = l.size();
  dataStore(stream, size, nullptr);

  typename std::deque<T>::iterator it = l.begin();
  typename std::deque<T>::iterator end = l.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    dataStore(stream, x, context);
  }
}

template <typename T, typename U>
inline void
dataStore(std::ostream & stream, std::map<T, U> & m, void * context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename std::map<T, U>::iterator it = m.begin();
  typename std::map<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    dataStore(stream, key, context);

    dataStore(stream, it->second, context);
  }
}

template <typename T, typename U>
inline void
dataStore(std::ostream & stream, std::unordered_map<T, U> & m, void * context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename std::unordered_map<T, U>::iterator it = m.begin();
  typename std::unordered_map<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    dataStore(stream, key, context);

    dataStore(stream, it->second, context);
  }
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::unordered_set<T> & s, void * context)
{
  // First store the size of the set
  std::size_t size = s.size();
  dataStore(stream, size, nullptr);

  for (auto & element : s)
    dataStore(stream, element, context);
}

template <typename T>
inline void
dataStore(std::ostream & stream, std::optional<T> & m, void * context)
{
  bool has_value = m.has_value();
  dataStore(stream, has_value, nullptr);

  if (has_value)
    dataStore(stream, *m, context);
}

template <typename T, typename U>
inline void
dataStore(std::ostream & stream, HashMap<T, U> & m, void * context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename HashMap<T, U>::iterator it = m.begin();
  typename HashMap<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    dataStore(stream, key, context);

    dataStore(stream, it->second, context);
  }
}

template <typename T, int Rows, int Cols>
void
dataStore(std::ostream & stream, Eigen::Matrix<T, Rows, Cols> & v, void * context)
{
  auto m = cast_int<unsigned int>(v.rows());
  dataStore(stream, m, context);
  auto n = cast_int<unsigned int>(v.cols());
  dataStore(stream, n, context);
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
    {
      auto & r = v(i, j);
      dataStore(stream, r, context);
    }
}

template <typename T>
void
dataStore(std::ostream & stream, GenericTwoVector<T> & v, void * context)
{
  dataStore(stream, static_cast<Eigen::Matrix<T, 2, 1> &>(v), context);
}

template <typename T, std::size_t N>
void
dataStore(std::ostream & stream, T (&arr)[N], void * context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataStore(stream, arr[i], context);
}

template <typename T, std::size_t N>
void
dataStore(std::ostream & stream, std::array<T, N> & arr, void * context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataStore(stream, arr[i], context);
}

template <typename T>
void
dataStore(std::ostream & stream, libMesh::NumericVector<T> & v, void * context)
{
  v.close();

  numeric_index_type size = v.local_size();

  for (numeric_index_type i = v.first_local_index(); i < v.first_local_index() + size; i++)
  {
    T r = v(i);
    dataStore(stream, r, context);
  }
}

template <>
void dataStore(std::ostream & stream, Vec & v, void * context);

template <typename T>
void
dataStore(std::ostream & stream, DenseVector<T> & v, void * context)
{
  unsigned int m = v.size();
  dataStore(stream, m, nullptr);
  for (unsigned int i = 0; i < v.size(); i++)
  {
    T r = v(i);
    dataStore(stream, r, context);
  }
}

template <typename T>
void
dataStore(std::ostream & stream, RankTwoTensorTempl<T> & rtt, void * context)
{
  dataStore(stream, rtt._coords, context);
}

template <typename T>
void
dataStore(std::ostream & stream, RankThreeTensorTempl<T> & rtt, void * context)
{
  dataStore(stream, rtt._vals, context);
}

template <typename T>
void
dataStore(std::ostream & stream, RankFourTensorTempl<T> & rft, void * context)
{
  dataStore(stream, rft._vals, context);
}

template <typename T>
void
dataStore(std::ostream & stream, SymmetricRankTwoTensorTempl<T> & srtt, void * context)
{
  dataStore(stream, srtt._vals, context);
}

template <typename T>
void
dataStore(std::ostream & stream, SymmetricRankFourTensorTempl<T> & srft, void * context)
{
  dataStore(stream, srft._vals, context);
}

template <typename T>
void
dataStore(std::ostream & stream, ColumnMajorMatrixTempl<T> & cmm, void * context)
{
  dataStore(stream, cmm._values, context);
}

/**
 * UniqueStorage helper routine
 *
 * The data within the UniqueStorage object cannot be null. The helper
 * for unique_ptr<T> is called to store the data.
 */
template <typename T>
void
dataStore(std::ostream & stream, UniqueStorage<T> & data, void * context)
{
  std::size_t size = data.size();
  dataStore(stream, size, nullptr);

  for (const auto i : index_range(data))
  {
    mooseAssert(data.hasValue(i), "Data doesn't have a value");
    dataStore(stream, data.pointerValue(i), context);
  }
}

// DO NOT MODIFY THE NEXT LINE - It is used by MOOSEDocs
// *************** Global Load Declarations *****************
template <typename T>
inline void
dataLoad(std::istream &, T &, void * /*context*/)
{
  static_assert(sizeof(T) == 0, "Must write dataLoad specialization");
}

namespace moose
{
namespace internal
{
template <typename T>
void
dataLoadSizeof(std::istream & stream, T & v)
{
  stream.read((char *)&v, sizeof(v));
  mooseAssert(!stream.bad(), "Failed to load");
}
}
}

// clang-format off
#define dataLoadSizeofSpecialization(Type) \
template <> \
inline void \
dataLoad(std::istream & stream, Type & v, void *) \
{ \
  moose::internal::dataLoadSizeof(stream, v); \
}
// template <>
// inline void
// dataLoad(std::istream & stream, const Type & v, void *)
// {
//   moose::internal::dataLoadSizeof(stream, v);
// }

dataLoadSizeofSpecialization(bool)
dataLoadSizeofSpecialization(char)
dataLoadSizeofSpecialization(short int)
dataLoadSizeofSpecialization(int)
dataLoadSizeofSpecialization(long int)
dataLoadSizeofSpecialization(long long int)
dataLoadSizeofSpecialization(unsigned char)
dataLoadSizeofSpecialization(short unsigned int)
dataLoadSizeofSpecialization(unsigned int)
dataLoadSizeofSpecialization(long unsigned int)
dataLoadSizeofSpecialization(long long unsigned int)
dataLoadSizeofSpecialization(float)
dataLoadSizeofSpecialization(double)
dataLoadSizeofSpecialization(long double)

#define dataLoadEnum(EnumType, IntType) \
template <> \
inline void dataLoad(std::istream & stream, EnumType & enum_type, void * ctx) \
{ \
  IntType loaded; \
  dataLoad(stream, loaded, ctx); \
  enum_type = static_cast<EnumType>(loaded); \
}

dataLoadEnum(libMesh::FEFamily, int)
dataLoadEnum(libMesh::ParallelType, int)
dataLoadEnum(Moose::CoordinateSystemType, int)

template <>
void dataLoad(std::istream &, mt_state &, void *);
// clang-format on

// Specializations (defined in .C)
template <>
void dataLoad(std::istream & stream, std::string & v, void * /*context*/);
template <>
void dataLoad(std::istream & stream, VariableName & v, void * /*context*/);
template <>
void dataLoad(std::istream & stream, UserObjectName & v, void * /*context*/);
template <>
void dataLoad(std::istream & stream, libMesh::FEType & v, void * /*context*/);
template <>
void dataLoad(std::istream & stream, const Elem *& e, void * context);
template <>
void dataLoad(std::istream & stream, const Node *& e, void * context);
template <>
void dataLoad(std::istream & stream, Elem *& e, void * context);
template <>
void dataLoad(std::istream & stream, Node *& e, void * context);
template <>
void dataLoad(std::istream & stream, std::stringstream & s, void * context);
template <>
void dataLoad(std::istream & stream, ADReal & dn, void * context);
template <>
void dataLoad(std::istream & stream, libMesh::Parameters & p, void * context);
template <>
void dataLoad(std::istream & stream, libMesh::Point & p, void * context);

template <>
/**
 * Loads an owned numeric vector.
 *
 * This is used in lieu of the NumericVector<double> & implementation when
 * the vector may not necessarily be initialized yet on the loading of the data.
 *
 * If \p is not null, it must have the same global and local sizes that it
 * was stored with. In this case, the data is simply filled into the vector.
 *
 * If \p is null, it will be constructed with the type (currently just a
 * PetscVector) stored and initialized with the global and local sizes stored.
 * The data will then be filled after initialization.
 *
 * Requirements: the vector cannot be ghosted, the provided context must be
 * the Communicator, and if \p v is initialized, it must have the same global
 * and local sizes that the vector was stored with.
 */
void dataLoad(std::istream & stream,
              std::unique_ptr<libMesh::NumericVector<libMesh::Number>> & v,
              void * context);

template <typename T>
void dataLoad(std::istream & stream, std::shared_ptr<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::unique_ptr<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, libMesh::TensorValue<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, libMesh::DenseMatrix<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, libMesh::DenseVector<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, libMesh::VectorValue<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::vector<T> & v, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::set<T> & s, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::list<T> & l, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::deque<T> & l, void * context);

template <typename T, typename U>
void dataLoad(std::istream & stream, std::map<T, U> & m, void * context);

template <typename T, typename U>
void dataLoad(std::istream & stream, std::unordered_map<T, U> & m, void * context);

template <typename T>
void dataLoad(std::istream & stream, std::unordered_set<T> & s, void * context);

template <typename T, std::size_t N>
void dataLoad(std::istream & stream, std::array<T, N> & arr, void * context);

template <typename T, int Rows, int Cols>
void dataLoad(std::istream & stream, Eigen::Matrix<T, Rows, Cols> & v, void * context);

template <typename T>
void
dataLoad(std::istream & /*stream*/, T *& /*v*/, void * /*context*/)
{
  mooseError("Attempting to load a raw pointer type: \"",
             libMesh::demangle(typeid(T).name()),
             " *\" as restartable data!\nWrite a custom dataLoad() template specialization!\n\n");
}
// clang-format on

template <typename T, typename U>
inline void
dataLoad(std::istream & stream, std::pair<T, U> & p, void * context)
{
  dataLoad(stream, p.first, context);
  dataLoad(stream, p.second, context);
}

inline void
dataLoad(std::istream & stream, std::vector<bool> & v, void * context)
{
  // First read the size of the vector
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  v.resize(size);

  bool tmp{};
  for (unsigned int i = 0; i < size; i++)
  {
    dataLoad(stream, tmp, context);
    v[i] = tmp;
  }
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::vector<T> & v, void * context)
{
  // First read the size of the vector
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  v.resize(size);

  for (unsigned int i = 0; i < size; i++)
    dataLoad(stream, v[i], context);
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::shared_ptr<T> & v, void * context)
{
  T * tmp = v.get();

  dataLoad(stream, tmp, context);
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::unique_ptr<T> & v, void * context)
{
  T * tmp = v.get();

  dataLoad(stream, tmp, context);
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::set<T> & s, void * context)
{
  // First read the size of the set
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    dataLoad(stream, data, context);
    s.insert(std::move(data));
  }
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::list<T> & l, void * context)
{
  // First read the size of the set
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    dataLoad(stream, data, context);
    l.push_back(std::move(data));
  }
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::deque<T> & l, void * context)
{
  // First read the size of the container
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    dataLoad(stream, data, context);
    l.push_back(std::move(data));
  }
}

template <typename T, typename U>
inline void
dataLoad(std::istream & stream, std::map<T, U> & m, void * context)
{
  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    dataLoad(stream, key, context);

    U & value = m[key];
    dataLoad(stream, value, context);
  }
}

template <typename T, typename U>
inline void
dataLoad(std::istream & stream, std::unordered_map<T, U> & m, void * context)
{
  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    dataLoad(stream, key, context);

    U & value = m[key];
    dataLoad(stream, value, context);
  }
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::unordered_set<T> & s, void * context)
{
  s.clear();

  // First read the size of the set
  std::size_t size = 0;
  dataLoad(stream, size, nullptr);
  s.reserve(size);

  for (std::size_t i = 0; i < size; i++)
  {
    T element;
    dataLoad(stream, element, context);
    s.insert(element);
  }
}

template <typename T>
inline void
dataLoad(std::istream & stream, std::optional<T> & m, void * context)
{
  bool has_value;
  dataLoad(stream, has_value, nullptr);

  if (has_value)
  {
    m = T{};
    dataLoad(stream, *m, context);
  }
  else
    m.reset();
}

template <typename T, typename U>
inline void
dataLoad(std::istream & stream, HashMap<T, U> & m, void * context)
{
  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    dataLoad(stream, key, context);

    U & value = m[key];
    dataLoad(stream, value, context);
  }
}

template <typename T, int Rows, int Cols>
void
dataLoad(std::istream & stream, Eigen::Matrix<T, Rows, Cols> & v, void * context)
{
  unsigned int m = 0;
  dataLoad(stream, m, context);
  unsigned int n = 0;
  dataLoad(stream, n, context);
  v.resize(m, n);
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
    {
      T r{};
      dataLoad(stream, r, context);
      v(i, j) = r;
    }
}

template <typename T>
void
dataLoad(std::istream & stream, GenericTwoVector<T> & v, void * context)
{
  dataLoad(stream, static_cast<Eigen::Matrix<T, 2, 1> &>(v), context);
}

template <typename T, std::size_t N>
void
dataLoad(std::istream & stream, std::array<T, N> & arr, void * context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataLoad(stream, arr[i], context);
}

template <typename T, std::size_t N>
void
dataLoad(std::istream & stream, T (&arr)[N], void * context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataLoad(stream, arr[i], context);
}

template <typename T>
void
dataLoad(std::istream & stream, libMesh::NumericVector<T> & v, void * context)
{
  numeric_index_type size = v.local_size();
  for (numeric_index_type i = v.first_local_index(); i < v.first_local_index() + size; i++)
  {
    T r = 0;
    dataLoad(stream, r, context);
    v.set(i, r);
  }
  v.close();
}

template <>
void dataLoad(std::istream & stream, Vec & v, void * context);

template <typename T>
void
dataLoad(std::istream & stream, DenseVector<T> & v, void * context)
{
  unsigned int n = 0;
  dataLoad(stream, n, nullptr);
  v.resize(n);
  for (unsigned int i = 0; i < n; i++)
  {
    T r = 0;
    dataLoad(stream, r, context);
    v(i) = r;
  }
}

template <typename T>
void
dataLoad(std::istream & stream, RankTwoTensorTempl<T> & rtt, void * context)
{
  dataLoad(stream, rtt._coords, context);
}

template <typename T>
void
dataLoad(std::istream & stream, RankThreeTensorTempl<T> & rtt, void * context)
{
  dataLoad(stream, rtt._vals, context);
}

template <typename T>
void
dataLoad(std::istream & stream, RankFourTensorTempl<T> & rft, void * context)
{
  dataLoad(stream, rft._vals, context);
}

template <typename T>
void
dataLoad(std::istream & stream, SymmetricRankTwoTensorTempl<T> & rtt, void * context)
{
  dataLoad(stream, rtt._vals, context);
}

template <typename T>
void
dataLoad(std::istream & stream, SymmetricRankFourTensorTempl<T> & rft, void * context)
{
  dataLoad(stream, rft._vals, context);
}

template <typename T>
void
dataLoad(std::istream & stream, ColumnMajorMatrixTempl<T> & cmm, void * context)
{
  dataLoad(stream, cmm._values, context);
}

/**
 * UniqueStorage Helper Function
 *
 * The unique_ptr<T> loader is called to load the data. That is,
 * you will likely need a specialization of unique_ptr<T> that will
 * appropriately construct and then fill the piece of data.
 */
template <typename T>
inline void
dataLoad(std::istream & stream, UniqueStorage<T> & data, void * context)
{
  std::size_t size;
  dataLoad(stream, size, nullptr);
  data.resize(size);

  for (const auto i : index_range(data))
    dataLoad(stream, data.pointerValue(i), context);
}

#ifndef TIMPI_HAVE_STRING_PACKING
/**
 * The following methods are specializations for using the libMesh::Parallel::packed_range_*
 * routines
 * for std::strings. These are here because the dataLoad/dataStore routines create raw string
 * buffers that can be communicated in a standard way using packed ranges.
 */
namespace libMesh
{
namespace Parallel
{
template <typename T>
class Packing<std::basic_string<T>>
{
public:
  static const unsigned int size_bytes = 4;

  typedef T buffer_type;

  static unsigned int get_string_len(typename std::vector<T>::const_iterator in)
  {
    unsigned int string_len = reinterpret_cast<const unsigned char &>(in[size_bytes - 1]);
    for (signed int i = size_bytes - 2; i >= 0; --i)
    {
      string_len *= 256;
      string_len += reinterpret_cast<const unsigned char &>(in[i]);
    }
    return string_len;
  }

  static unsigned int packed_size(typename std::vector<T>::const_iterator in)
  {
    return get_string_len(in) + size_bytes;
  }

  static unsigned int packable_size(const std::basic_string<T> & s, const void *)
  {
    return s.size() + size_bytes;
  }

  template <typename Iter>
  static void pack(const std::basic_string<T> & b, Iter data_out, const void *)
  {
    unsigned int string_len = b.size();
    for (unsigned int i = 0; i != size_bytes; ++i)
    {
      *data_out++ = (string_len % 256);
      string_len /= 256;
    }

    std::copy(b.begin(), b.end(), data_out);
  }

  static std::basic_string<T> unpack(typename std::vector<T>::const_iterator in, void *)
  {
    unsigned int string_len = get_string_len(in);

    std::ostringstream oss;
    for (unsigned int i = 0; i < string_len; ++i)
      oss << reinterpret_cast<const unsigned char &>(in[i + size_bytes]);

    in += size_bytes + string_len;

    return oss.str();
  }
};

} // namespace Parallel

} // namespace libMesh

#endif
