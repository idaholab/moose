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
#include "AnyPointer.h"

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
class Point;
}

/**
 * Scalar helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, P & data, Context context);

/**
 * Vector helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, std::vector<P> & data, Context context);

/**
 * Shared pointer helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, std::shared_ptr<P> & data, Context context);

/**
 * Unique pointer helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, std::unique_ptr<P> & data, Context context);

/**
 * Set helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, std::set<P> & data, Context context);

/**
 * Map helper routine
 */
template <typename P, typename Q, typename Context>
inline void storeHelper(std::ostream & stream, std::map<P, Q> & data, Context context);

/**
 * Unordered_map helper routine
 */
template <typename P, typename Q, typename Context>
inline void storeHelper(std::ostream & stream, std::unordered_map<P, Q> & data, Context context);

/**
 * Optional helper routine
 */
template <typename P, typename Context>
inline void storeHelper(std::ostream & stream, std::optional<P> & data, Context context);

/**
 * HashMap helper routine
 */
template <typename P, typename Q, typename Context>
inline void storeHelper(std::ostream & stream, HashMap<P, Q> & data, Context context);

/**
 * UniqueStorage helper routine
 */
template <typename T, typename Context>
inline void storeHelper(std::ostream & stream, UniqueStorage<T> & data, Context context);

/**
 * Scalar helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, P & data, Context context);

/**
 * Vector helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, std::vector<P> & data, Context context);

/**
 * Shared Pointer helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, std::shared_ptr<P> & data, Context context);

/**
 * Unique Pointer helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, std::unique_ptr<P> & data, Context context);

/**
 * Set helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, std::set<P> & data, Context context);

/**
 * Map helper routine
 */
template <typename P, typename Q, typename Context>
inline void loadHelper(std::istream & stream, std::map<P, Q> & data, Context context);

/**
 * Unordered_map helper routine
 */
template <typename P, typename Q, typename Context>
inline void loadHelper(std::istream & stream, std::unordered_map<P, Q> & data, Context context);

/**
 * Optional helper routine
 */
template <typename P, typename Context>
inline void loadHelper(std::istream & stream, std::optional<P> & data, Context context);

/**
 * Hashmap helper routine
 */
template <typename P, typename Q, typename Context>
inline void loadHelper(std::istream & stream, HashMap<P, Q> & data, Context context);

/**
 * UniqueStorage helper routine
 */
template <typename T, typename Context>
inline void loadHelper(std::istream & stream, UniqueStorage<T> & data, Context context);

// Forward declaration with SFINAE - only enabled for trivially copyable, non-polymorphic types
template <typename T,
          typename Context,
          std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_polymorphic_v<T>, int> = 0>
inline void dataStore(std::ostream & stream, T & v, Context /*context*/);

// DO NOT MODIFY THE NEXT LINE - It is used by MOOSEDocs
// *************** Global Store Declarations *****************
/**
 * Generic dataStore for trivially copyable, non-polymorphic types.
 * Uses SFINAE to disable this template for other types, allowing explicit
 * specializations (like for std::string) to be selected when nullptr is passed.
 */
template <typename T,
          typename Context,
          std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_polymorphic_v<T>, int>>
inline void
dataStore(std::ostream & stream, T & v, Context /*context*/)
{
  stream.write((char *)&v, sizeof(v));
  mooseAssert(!stream.bad(), "Failed to store");
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & /*stream*/, T *& /*v*/, Context /*context*/)
{
  mooseError("Attempting to store a raw pointer type: \"",
             libMesh::demangle(typeid(T).name()),
             " *\" as restartable data!\nWrite a custom dataStore() template specialization!\n\n");
}

template <typename Context>
void dataStore(std::ostream & stream, Point & p, Context context);

template <typename T, typename U, typename Context>
inline void
dataStore(std::ostream & stream, std::pair<T, U> & p, Context context)
{
  storeHelper(stream, p.first, context);
  storeHelper(stream, p.second, context);
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::vector<T> & v, Context context)
{
  // First store the size of the vector
  unsigned int size = v.size();
  dataStore(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
    storeHelper(stream, v[i], context);
}

// Specialization for std::vector<bool> which uses proxy references
template <typename Context>
inline void
dataStore(std::ostream & stream, std::vector<bool> & v, Context /*context*/)
{
  unsigned int size = v.size();
  dataStore(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    bool val = v[i];
    dataStore(stream, val, nullptr);
  }
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::shared_ptr<T> & v, Context context)
{
  T * tmp = v.get();

  storeHelper(stream, tmp, context);
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::unique_ptr<T> & v, Context context)
{
  T * tmp = v.get();

  storeHelper(stream, tmp, context);
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::set<T> & s, Context context)
{
  // First store the size of the set
  unsigned int size = s.size();
  dataStore(stream, size, nullptr);

  typename std::set<T>::iterator it = s.begin();
  typename std::set<T>::iterator end = s.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    storeHelper(stream, x, context);
  }
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::list<T> & l, Context context)
{
  // First store the size of the set
  unsigned int size = l.size();
  dataStore(stream, size, nullptr);

  typename std::list<T>::iterator it = l.begin();
  typename std::list<T>::iterator end = l.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    storeHelper(stream, x, context);
  }
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::deque<T> & l, Context context)
{
  // First store the size of the container
  unsigned int size = l.size();
  dataStore(stream, size, nullptr);

  typename std::deque<T>::iterator it = l.begin();
  typename std::deque<T>::iterator end = l.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T &>(*it);
    storeHelper(stream, x, context);
  }
}

template <typename T, typename U, typename Context>
inline void
dataStore(std::ostream & stream, std::map<T, U> & m, Context context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename std::map<T, U>::iterator it = m.begin();
  typename std::map<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    storeHelper(stream, key, context);

    storeHelper(stream, it->second, context);
  }
}

template <typename T, typename U, typename Context>
inline void
dataStore(std::ostream & stream, std::unordered_map<T, U> & m, Context context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename std::unordered_map<T, U>::iterator it = m.begin();
  typename std::unordered_map<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    storeHelper(stream, key, context);

    storeHelper(stream, it->second, context);
  }
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::unordered_set<T> & s, Context context)
{
  // First store the size of the set
  std::size_t size = s.size();
  dataStore(stream, size, nullptr);

  for (auto & element : s)
    dataStore(stream, element, context);
}

template <typename T, typename Context>
inline void
dataStore(std::ostream & stream, std::optional<T> & m, Context context)
{
  bool has_value = m.has_value();
  dataStore(stream, has_value, nullptr);

  if (has_value)
    storeHelper(stream, *m, context);
}

template <typename T, typename U, typename Context>
inline void
dataStore(std::ostream & stream, HashMap<T, U> & m, Context context)
{
  // First store the size of the map
  unsigned int size = m.size();
  dataStore(stream, size, nullptr);

  typename HashMap<T, U>::iterator it = m.begin();
  typename HashMap<T, U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    T & key = const_cast<T &>(it->first);

    storeHelper(stream, key, context);

    storeHelper(stream, it->second, context);
  }
}

// Templated overloads for specific types
// IMPORTANT: These must be declared BEFORE templates that may call them
// (like Eigen::Matrix) for two-phase lookup to work correctly.
template <typename Context>
void dataStore(std::ostream & stream, Real & v, Context context);
template <typename Context>
void dataStore(std::ostream & stream, std::string & v, Context context);
template <typename Context>
void dataStore(std::ostream & stream, VariableName & v, Context context);
template <typename Context>
void dataStore(std::ostream & stream, UserObjectName & v, Context context);
template <typename Context>
void dataStore(std::ostream & stream, bool & v, Context context);
// Vectors of bools are special
// https://en.wikipedia.org/w/index.php?title=Sequence_container_(C%2B%2B)&oldid=767869909#Specialization_for_bool
template <typename Context>
void dataStore(std::ostream & stream, std::vector<bool> & v, Context context);
// Elem/Node store: accepts any context (just stores ID, ignores context)
template <typename Context>
void dataStore(std::ostream & stream, const Elem *& e, Context context);
template <typename Context>
void dataStore(std::ostream & stream, const Node *& n, Context context);
template <typename Context>
void dataStore(std::ostream & stream, Elem *& e, Context context);
template <typename Context>
void dataStore(std::ostream & stream, Node *& n, Context context);
template <typename Context>
void dataStore(std::ostream & stream, std::stringstream & s, Context context);
template <typename Context>
void dataStore(std::ostream & stream, ADReal & dn, Context context);
template <typename Context>
void dataStore(std::ostream & stream, libMesh::Parameters & p, Context context);
// nlohmann::json dataStore/dataLoad (defined in JSONOutput.C)
template <typename Context>
void dataStore(std::ostream & stream, nlohmann::json & json, Context context);
template <typename Context>
void dataLoad(std::istream & stream, nlohmann::json & json, Context context);

template <typename T, int Rows, int Cols, typename Context>
void
dataStore(std::ostream & stream, Eigen::Matrix<T, Rows, Cols> & v, Context context)
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

template <typename T, typename Context>
void
dataStore(std::ostream & stream, GenericTwoVector<T> & v, Context context)
{
  dataStore(stream, static_cast<Eigen::Matrix<T, 2, 1> &>(v), context);
}

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
               const libMesh::Parallel::Communicator * context);

template <std::size_t N, typename Context>
inline void
dataStore(std::ostream & stream, std::array<ADReal, N> & dn, Context context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataStore(stream, dn[i], context);
}

template <std::size_t N, typename Context>
inline void
dataStore(std::ostream & stream, ADReal (&dn)[N], Context context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataStore(stream, dn[i], context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, libMesh::NumericVector<T> & v, Context context)
{
  v.close();

  numeric_index_type size = v.local_size();

  for (numeric_index_type i = v.first_local_index(); i < v.first_local_index() + size; i++)
  {
    T r = v(i);
    dataStore(stream, r, context);
  }
}

template <typename Context>
void dataStore(std::ostream & stream, Vec & v, Context context);

template <typename T, typename Context>
void
dataStore(std::ostream & stream, DenseVector<T> & v, Context context)
{
  unsigned int m = v.size();
  dataStore(stream, m, nullptr);
  for (unsigned int i = 0; i < v.size(); i++)
  {
    T r = v(i);
    dataStore(stream, r, context);
  }
}

template <typename T, typename Context>
void dataStore(std::ostream & stream, libMesh::TensorValue<T> & v, Context context);

template <typename T, typename Context>
void dataStore(std::ostream & stream, libMesh::DenseMatrix<T> & v, Context context);

template <typename T, typename Context>
void dataStore(std::ostream & stream, libMesh::VectorValue<T> & v, Context context);

template <typename T, typename Context>
void
dataStore(std::ostream & stream, RankTwoTensorTempl<T> & rtt, Context context)
{
  dataStore(stream, rtt._coords, context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, RankThreeTensorTempl<T> & rtt, Context context)
{
  dataStore(stream, rtt._vals, context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, RankFourTensorTempl<T> & rft, Context context)
{
  dataStore(stream, rft._vals, context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, SymmetricRankTwoTensorTempl<T> & srtt, Context context)
{
  dataStore(stream, srtt._vals, context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, SymmetricRankFourTensorTempl<T> & srft, Context context)
{
  dataStore(stream, srft._vals, context);
}

template <typename T, typename Context>
void
dataStore(std::ostream & stream, ColumnMajorMatrixTempl<T> & cmm, Context context)
{
  dataStore(stream, cmm._values, context);
}

// DO NOT MODIFY THE NEXT LINE - It is used by MOOSEDocs
// *************** Global Load Declarations *****************
/**
 * Generic dataLoad for trivially copyable, non-polymorphic types.
 * Uses SFINAE to disable this template for other types, allowing explicit
 * specializations (like for std::string) to be selected when nullptr is passed.
 */
template <typename T,
          typename Context,
          std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_polymorphic_v<T>, int> = 0>
inline void
dataLoad(std::istream & stream, T & v, Context /*context*/)
{
  stream.read((char *)&v, sizeof(v));
  mooseAssert(!stream.bad(), "Failed to load");
}

// Specializations for libtorch neural network pointers
// These pointers reference objects managed elsewhere and are reconstructed after restart
#ifdef MOOSE_LIBTORCH_ENABLED
namespace Moose
{
class LibtorchArtificialNeuralNet;
}

template <typename Context>
inline void
dataStore(std::ostream & stream, const Moose::LibtorchArtificialNeuralNet * const & ptr, Context)
{
  // Store a null sentinel - the pointer will be reconstructed in initialSetup/execute
  char null_flag = (ptr == nullptr) ? 1 : 0;
  dataStore(stream, null_flag, nullptr);
}

template <typename Context>
inline void
dataLoad(std::istream & stream, const Moose::LibtorchArtificialNeuralNet *& ptr, Context)
{
  // Load the sentinel and set to nullptr - will be reconstructed later
  char null_flag;
  dataLoad(stream, null_flag, nullptr);
  ptr = nullptr;
}
#endif

template <typename T, typename U, typename Context>
inline void
dataLoad(std::istream & stream, std::pair<T, U> & p, Context context)
{
  loadHelper(stream, p.first, context);
  loadHelper(stream, p.second, context);
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::vector<T> & v, Context context)
{
  // First read the size of the vector
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  v.resize(size);

  for (unsigned int i = 0; i < size; i++)
    loadHelper(stream, v[i], context);
}

// Specialization for std::vector<bool> which uses proxy references
template <typename Context>
inline void
dataLoad(std::istream & stream, std::vector<bool> & v, Context /*context*/)
{
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  v.resize(size);

  for (unsigned int i = 0; i < size; i++)
  {
    bool val;
    dataLoad(stream, val, nullptr);
    v[i] = val;
  }
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::shared_ptr<T> & v, Context context)
{
  T * tmp = v.get();

  loadHelper(stream, tmp, context);
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::unique_ptr<T> & v, Context context)
{
  T * tmp = v.get();

  loadHelper(stream, tmp, context);
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::set<T> & s, Context context)
{

  // First read the size of the set
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    loadHelper(stream, data, context);
    s.insert(std::move(data));
  }
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::list<T> & l, Context context)
{

  // First read the size of the set
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data{};
    loadHelper(stream, data, context);
    l.push_back(std::move(data));
  }
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::deque<T> & l, Context context)
{

  // First read the size of the container
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    loadHelper(stream, data, context);
    l.push_back(std::move(data));
  }
}

template <typename T, typename U, typename Context>
inline void
dataLoad(std::istream & stream, std::map<T, U> & m, Context context)
{

  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U & value = m[key];
    loadHelper(stream, value, context);
  }
}

template <typename T, typename U, typename Context>
inline void
dataLoad(std::istream & stream, std::unordered_map<T, U> & m, Context context)
{

  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U & value = m[key];
    loadHelper(stream, value, context);
  }
}

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::unordered_set<T> & s, Context context)
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

template <typename T, typename Context>
inline void
dataLoad(std::istream & stream, std::optional<T> & m, Context context)
{

  bool has_value;
  dataLoad(stream, has_value, nullptr);

  if (has_value)
  {
    m = T{};
    loadHelper(stream, *m, context);
  }
  else
    m.reset();
}

template <typename T, typename U, typename Context>
inline void
dataLoad(std::istream & stream, HashMap<T, U> & m, Context context)
{

  // First read the size of the map
  unsigned int size = 0;
  dataLoad(stream, size, nullptr);

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U & value = m[key];
    loadHelper(stream, value, context);
  }
}

// Templated overloads for specific types
// IMPORTANT: These must be declared BEFORE templates that may call them
// (like Eigen::Matrix) for two-phase lookup to work correctly.
template <typename Context>
void dataLoad(std::istream & stream, Real & v, Context context);
template <typename Context>
void dataLoad(std::istream & stream, std::string & v, Context context);
template <typename Context>
void dataLoad(std::istream & stream, VariableName & v, Context context);
template <typename Context>
void dataLoad(std::istream & stream, UserObjectName & v, Context context);
template <typename Context>
void dataLoad(std::istream & stream, bool & v, Context context);
// Vectors of bools are special
// https://en.wikipedia.org/w/index.php?title=Sequence_container_(C%2B%2B)&oldid=767869909#Specialization_for_bool
template <typename Context>
void dataLoad(std::istream & stream, std::vector<bool> & v, Context context);
// Elem/Node load: REQUIRES MeshBase* context
void dataLoad(std::istream & stream, const Elem *& e, libMesh::MeshBase * context);
void dataLoad(std::istream & stream, const Node *& n, libMesh::MeshBase * context);
void dataLoad(std::istream & stream, Elem *& e, libMesh::MeshBase * context);
void dataLoad(std::istream & stream, Node *& n, libMesh::MeshBase * context);
template <typename Context>
void dataLoad(std::istream & stream, std::stringstream & s, Context context);
template <typename Context>
void dataLoad(std::istream & stream, ADReal & dn, Context context);
template <typename Context>
void dataLoad(std::istream & stream, libMesh::Parameters & p, Context context);

template <typename T, int Rows, int Cols, typename Context>
void
dataLoad(std::istream & stream, Eigen::Matrix<T, Rows, Cols> & v, Context context)
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

template <typename T, typename Context>
void
dataLoad(std::istream & stream, GenericTwoVector<T> & v, Context context)
{
  dataLoad(stream, static_cast<Eigen::Matrix<T, 2, 1> &>(v), context);
}
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
              const libMesh::Parallel::Communicator * context);

template <std::size_t N, typename Context>
inline void
dataLoad(std::istream & stream, std::array<ADReal, N> & dn, Context context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataLoad(stream, dn[i], context);
}

template <std::size_t N, typename Context>
inline void
dataLoad(std::istream & stream, ADReal (&dn)[N], Context context)
{
  for (std::size_t i = 0; i < N; ++i)
    dataLoad(stream, dn[i], context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, libMesh::NumericVector<T> & v, Context context)
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

template <typename Context>
void dataLoad(std::istream & stream, Vec & v, Context context);

template <typename T, typename Context>
void
dataLoad(std::istream & stream, DenseVector<T> & v, Context context)
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

template <typename T, typename Context>
void dataLoad(std::istream & stream, libMesh::TensorValue<T> & v, Context context);

template <typename T, typename Context>
void dataLoad(std::istream & stream, libMesh::DenseMatrix<T> & v, Context context);

template <typename T, typename Context>
void dataLoad(std::istream & stream, libMesh::VectorValue<T> & v, Context context);

template <typename T, typename Context>
void
dataLoad(std::istream & stream, RankTwoTensorTempl<T> & rtt, Context context)
{
  dataLoad(stream, rtt._coords, context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, RankThreeTensorTempl<T> & rtt, Context context)
{
  dataLoad(stream, rtt._vals, context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, RankFourTensorTempl<T> & rft, Context context)
{
  dataLoad(stream, rft._vals, context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, SymmetricRankTwoTensorTempl<T> & rtt, Context context)
{
  dataLoad(stream, rtt._vals, context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, SymmetricRankFourTensorTempl<T> & rft, Context context)
{
  dataLoad(stream, rft._vals, context);
}

template <typename T, typename Context>
void
dataLoad(std::istream & stream, ColumnMajorMatrixTempl<T> & cmm, Context context)
{
  dataLoad(stream, cmm._values, context);
}

// Scalar Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, P & data, Context context)
{
  dataStore(stream, data, context);
}

// Vector Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, std::vector<P> & data, Context context)
{
  dataStore(stream, data, context);
}

// std::shared_ptr Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, std::shared_ptr<P> & data, Context context)
{
  dataStore(stream, data, context);
}

// std::unique Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, std::unique_ptr<P> & data, Context context)
{
  dataStore(stream, data, context);
}

// Set Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, std::set<P> & data, Context context)
{
  dataStore(stream, data, context);
}

// Map Helper Function
template <typename P, typename Q, typename Context>
inline void
storeHelper(std::ostream & stream, std::map<P, Q> & data, Context context)
{
  dataStore(stream, data, context);
}

// Unordered_map Helper Function
template <typename P, typename Q, typename Context>
inline void
storeHelper(std::ostream & stream, std::unordered_map<P, Q> & data, Context context)
{
  dataStore(stream, data, context);
}

// Optional Helper Function
template <typename P, typename Context>
inline void
storeHelper(std::ostream & stream, std::optional<P> & data, Context context)
{
  dataStore(stream, data, context);
}

// HashMap Helper Function
template <typename P, typename Q, typename Context>
inline void
storeHelper(std::ostream & stream, HashMap<P, Q> & data, Context context)
{
  dataStore(stream, data, context);
}

/**
 * UniqueStorage helper routine
 *
 * The data within the UniqueStorage object cannot be null. The helper
 * for unique_ptr<T> is called to store the data.
 */
template <typename T, typename Context>
inline void
storeHelper(std::ostream & stream, UniqueStorage<T> & data, Context context)
{
  std::size_t size = data.size();
  dataStore(stream, size, nullptr);

  for (const auto i : index_range(data))
  {
    mooseAssert(data.hasValue(i), "Data doesn't have a value");
    storeHelper(stream, data.pointerValue(i), context);
  }
}

// Scalar Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, P & data, Context context)
{
  dataLoad(stream, data, context);
}

// Vector Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, std::vector<P> & data, Context context)
{
  dataLoad(stream, data, context);
}

// std::shared_ptr Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, std::shared_ptr<P> & data, Context context)
{
  dataLoad(stream, data, context);
}

// Unique Pointer Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, std::unique_ptr<P> & data, Context context)
{
  dataLoad(stream, data, context);
}

// Set Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, std::set<P> & data, Context context)
{
  dataLoad(stream, data, context);
}

// Map Helper Function
template <typename P, typename Q, typename Context>
inline void
loadHelper(std::istream & stream, std::map<P, Q> & data, Context context)
{
  dataLoad(stream, data, context);
}

// Unordered_map Helper Function
template <typename P, typename Q, typename Context>
inline void
loadHelper(std::istream & stream, std::unordered_map<P, Q> & data, Context context)
{
  dataLoad(stream, data, context);
}

// Optional Helper Function
template <typename P, typename Context>
inline void
loadHelper(std::istream & stream, std::optional<P> & data, Context context)
{
  dataLoad(stream, data, context);
}

// HashMap Helper Function
template <typename P, typename Q, typename Context>
inline void
loadHelper(std::istream & stream, HashMap<P, Q> & data, Context context)
{
  dataLoad(stream, data, context);
}

/**
 * UniqueStorage Helper Function
 *
 * The unique_ptr<T> loader is called to load the data. That is,
 * you will likely need a specialization of unique_ptr<T> that will
 * appropriately construct and then fill the piece of data.
 */
template <typename T, typename Context>
inline void
loadHelper(std::istream & stream, UniqueStorage<T> & data, Context context)
{
  std::size_t size;
  dataLoad(stream, size, nullptr);
  data.resize(size);

  for (const auto i : index_range(data))
    loadHelper(stream, data.pointerValue(i), context);
}

template <typename Context>
void dataLoad(std::istream & stream, Point & p, Context context);

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

  template <typename Context>
  static unsigned int packable_size(const std::basic_string<T> & s, Context)
  {
    return s.size() + size_bytes;
  }

  template <typename Iter, typename Context>
  static void pack(const std::basic_string<T> & b, Iter data_out, Context)
  {
    unsigned int string_len = b.size();
    for (unsigned int i = 0; i != size_bytes; ++i)
    {
      *data_out++ = (string_len % 256);
      string_len /= 256;
    }

    std::copy(b.begin(), b.end(), data_out);
  }

  template <typename Context>
  static std::basic_string<T> unpack(typename std::vector<T>::const_iterator in, Context)
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
