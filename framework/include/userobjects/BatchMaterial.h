//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

#include "libmesh/int_range.h"

#include <tuple>
#include <string>
#include <vector>
#include <utility>

namespace BatchMaterialUtils
{

// type wrappers
template <typename T>
struct GatherMatProp
{
  typedef T type;
  typedef const MaterialProperty<T> gather_type;

  template <typename M>
  static gather_type * getPointer(M & mpi, const MaterialPropertyName & name)
  {
    return &mpi.template getMaterialProperty<T>(name);
  }
};

struct GatherVariable
{
  typedef Real type;
  typedef const VariableValue gather_type;

  template <typename C>
  static gather_type * getPointer(const C & coupleable, const VariableName & name)
  {
    return &coupleable.coupledValue(name);
  }
};

// tuple wrappers
struct TupleStd
{
  // the tuple type
  template <typename... Args>
  using type = std::tuple<Args...>;

  // the element access function
  template <int N, typename T>
  static auto & get(T && t)
  {
    return std::get<N>(t);
  }

  // tuple size (number of elements)
  template <typename T>
  using size = std::tuple_size<T>;

  // element type access
  template <int I, typename T>
  using element = typename std::tuple_element<I, T>::type;
};

// this wrapper is yet untested
#ifdef CUDA_SUPPORTED
struct TupleCuda
{
  // the tuple type
  template <typename... Args>
  using type = cuda::std::tuple<Args...>;

  // the element access function
  template <int N, typename T>
  static auto & get(T && t)
  {
    return cuda::std::get<N>(t);
  }

  // tuple size (number of elements)
  template <typename T>
  using size = cuda::std::tuple_size<T>;

  // element type access
  template <int I, typename T>
  using element = typename cuda::std::tuple_element<I, T>::type;
};
#endif

}
template <typename Tuple, typename Output, typename... Input>
class BatchMaterial : public ElementUserObject
{
  /// shorthand for the instantiated template type
  typedef BatchMaterial<Tuple, Output, Input...> BatchMaterialType;

  /// Helper template for obtainig all variable and material property references
  template <int I, typename T, typename... Names>
  void construct(T && name, Names &&... names);

  /// Helper template for gathering all items into the input data tuples
  template <int I>
  void copy(const unsigned int nqp);

public:
  static InputParameters validParams() { return ElementUserObject::validParams(); }

  template <typename... Names>
  BatchMaterial(const InputParameters & params, Names &&... names) : ElementUserObject(params)
  {
    construct<0, Names...>(std::forward<Names>(names)...);
  }

  /// override this method to implement the computation on the batch data
  virtual void batchCompute() = 0;

  /// The output type is directly specified as a template parameter
  typedef Output OutputType;
  /// The serialized batch data is stored in a vector
  typedef std::vector<OutputType> OutputVector;

  /// input data needs to use a flexible tuple type (std::tuple or cuda::std::tuple)
  typedef typename Tuple::template type<typename Input::type...> InputType;
  /// The serialized batch data is stored in a vector
  typedef std::vector<InputType> InputVector;

  /// get a reference to the output data
  const OutputVector & getOutputData() const { return _output_data; }

  /// get a reference to the input data
  const InputVector & getInputData() const { return _input_data; }

  /// check if the output is fully computed and ready to be fetched
  bool outputReady() const { return _output_ready; }

  /// get the index for the first qp of a specified element (other qps are at the following indices)
  std::size_t getIndex(dof_id_type elem_id) const;

  /// data structures are initialized here
  void initialize() override final;

  /// All data is automatically gathered here
  void execute() override final;

  /// Assemble data from all threads
  void threadJoin(const UserObject & uo) override final;

  /// we call batchCompute() from here
  void finalize() override final;

  /// Input data, utilized in batchCompute()
  InputVector _input_data;

  /// Output data, written to in batchCompute()
  OutputVector _output_data;

  // use regular tuple for the Moose pointers
  std::tuple<typename Input::gather_type *...> _input_ptr;

  /// current element index
  std::size_t _index;

  /// map from element ID to index
  std::map<dof_id_type, std::size_t> _index_map;

  friend struct BatchMaterialUtils::GatherVariable;

private:
  /// flag that indicates if _output_data has been fully computed
  bool _output_ready;
};

template <typename Tuple, typename Output, typename... Input>
template <int I, typename T, typename... Names>
void
BatchMaterial<Tuple, Output, Input...>::construct(T && name, Names &&... names)
{
  // couple the variable or material property
  typedef typename std::tuple_element<I, std::tuple<Input...>>::type CurrentElement;
  std::get<I>(_input_ptr) = CurrentElement::getPointer(*this, name);

  // recursively couple the next variable or material property
  if constexpr (sizeof...(Names) > 0)
    construct<I + 1, Names...>(std::forward<Names>(names)...);
}

template <typename Tuple, typename Output, typename... Input>
template <int I>
void
BatchMaterial<Tuple, Output, Input...>::copy(const unsigned int nqp)
{
  // copy all qp values for the current item
  for (const auto qp : make_range(nqp))
    Tuple::template get<I>(_input_data[_index + qp]) = (*std::get<I>(_input_ptr))[qp];

  // proceed to next item
  if constexpr (I + 1 < sizeof...(Input))
    copy<I + 1>(nqp);
}

template <typename Tuple, typename Output, typename... Input>
std::size_t
BatchMaterial<Tuple, Output, Input...>::getIndex(dof_id_type elem_id) const
{
  const auto it = _index_map.find(elem_id);
  if (it == _index_map.end())
    mooseError("Element ", elem_id, " was not found in the index map for ", name(), ".");
  return it->second;
}

template <typename Tuple, typename Output, typename... Input>
void
BatchMaterial<Tuple, Output, Input...>::initialize()
{
  _index = 0;
  _output_ready = false;
}

template <typename Tuple, typename Output, typename... Input>
void
BatchMaterial<Tuple, Output, Input...>::execute()
{
  // update index map
  _index_map[_current_elem->id()] = _index;

  // make sure the input data is sized sufficiently big
  const auto nqp = _qrule->n_points();
  if (_input_data.size() < _index + nqp)
    _input_data.resize(_index + nqp);

  // copy data
  copy<0>(nqp);
  _index += nqp;
}

template <typename Tuple, typename Output, typename... Input>
void
BatchMaterial<Tuple, Output, Input...>::threadJoin(const UserObject & uo)
{
  // join maps (with index shift)
  const auto & bm = static_cast<const BatchMaterialType &>(uo);
  for (const auto & [id, index] : bm._index_map)
    _index_map[id] = index + _index;

  // make sure we have enough space
  const auto capacity = _input_data.capacity();
  if (capacity < _index + bm._index)
    _input_data.reserve(_index + bm._index);

  // concatenate input data
  _input_data.insert(std::begin(_input_data) + _index,
                     std::begin(bm._input_data),
                     std::begin(bm._input_data) + bm._index);
  _index += bm._index;
}

template <typename Tuple, typename Output, typename... Input>
void
BatchMaterial<Tuple, Output, Input...>::finalize()
{
  // resize the input and output data blocks to contain just the gathered items
  // (should be a no-op mostly)
  _input_data.resize(_index);
  _output_data.resize(_index);
  batchCompute();

  _output_ready = true;
}
