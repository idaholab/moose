#pragma once

#include "MooseError.h"

// nested initializer list

template <typename T, std::size_t depth>
struct NestedInitializerListTempl
{
  using type = std::initializer_list<typename NestedInitializerListTempl<T, depth - 1>::type>;
};

template <typename T>
struct NestedInitializerListTempl<T, 0>
{
  using type = T;
};

template <typename T, std::size_t depth>
using NestedInitializerList = typename NestedInitializerListTempl<T, depth>::type;

// tensor class

template <typename T>
class TensorTempl
{
public:
  /// zero scalar
  TensorTempl() : _data(1, T(0.0)), _shape(1, 1) {}

  /// scalar with value t
  TensorTempl(T t);

  /// up to order four tensors, convenience constructors
  TensorTempl(NestedInitializerList<T, 1> t) : _i(0) { initialize(t, 0); }
  TensorTempl(NestedInitializerList<T, 2> t) : _i(0) { initialize(t, 0); }
  TensorTempl(NestedInitializerList<T, 3> t) : _i(0) { initialize(t, 0); }
  TensorTempl(NestedInitializerList<T, 4> t) : _i(0) { initialize(t, 0); }

  // this sadly does not work:
  // template <int D> TensorTempl(NestedInitializerList<T, D> t) {}

  // operators
  template <typename... U>
  T operator()(U... index);
  T operator()(std::vector<int> index_caught);

  TensorTempl & operator+(const TensorTempl & rhs);
  TensorTempl & operator-(const TensorTempl & rhs);
  TensorTempl & operator*(const TensorTempl & rhs);
  TensorTempl & operator*(const T & rhs);
  friend TensorTempl & operator*(const T & lhs, TensorTempl<T> & rhs) { return rhs * lhs; }
  TensorTempl &
  contractMult(const TensorTempl & lhs, const TensorTempl & rhs, unsigned int contractor);
  void recurseMult(T & result_term,
                   unsigned int index,
                   unsigned int divider,
                   std::vector<long unsigned int> center,
                   unsigned int current,
                   std::vector<unsigned int> current_vec,
                   const TensorTempl & lhs,
                   const TensorTempl & rhs);

  friend std::ostream & operator<<(std::ostream & os, const TensorTempl<T> & tens)
  {
    os << "Shape : {";
    for (unsigned int i = 0; i < tens._shape.size(); ++i)
    {
      os << tens._shape[i];
      if (i != tens._shape.size() - 1)
        os << ',';
    }
    os << "}" << std::endl;
    os << "Accessor : {";
    for (unsigned int i = 0; i < tens._access_data.size(); ++i)
    {
      os << tens._access_data[i];
      if (i != tens._access_data.size() - 1)
        os << ',';
    }
    os << "}" << std::endl;
    os << "Data : {";
    for (unsigned int i = 0; i < tens._data.size(); ++i)
    {
      os << tens._data[i];
      if (i != tens._data.size() - 1)
        os << ',';
    }
    os << "}" << std::endl;
    return os;
  }

  /// works for order 1 and 2 only
  void transpose();
  void transpose(unsigned int first_index, unsigned int second_index);

  void setAccessor();

  void applyFunction(void (*function)(T & t));

protected:
  std::vector<T> _data;
  std::vector<std::size_t> _shape;
  std::vector<unsigned int> _access_data;

private:
  // initializer list processing
  template <typename U>
  void initialize(std::initializer_list<U> u, std::size_t _depth);
  void initialize(std::initializer_list<T> t, std::size_t _depth);
  void checkDimensions(std::size_t size, std::size_t _depth);

  // helper index
  std::size_t _i;
};

// operators
template <typename T>
template <typename... U>
T
TensorTempl<T>::operator()(U... index)
{
  std::vector<int> indices({index...});
  return (*this)(indices);
}

template <typename T>
T
TensorTempl<T>::operator()(std::vector<int> index_caught)
{
  unsigned int location = 0;
  if (index_caught.size() != _shape.size())
    mooseError("Incorrect number of indeces for accessing tensor.");
  for (unsigned int i = 0; i < _access_data.size(); ++i)
  {
    if (unsigned(index_caught[i]) >= _shape[i])
      mooseError("Incorrect indices for accessing tensor.");
    location += _access_data[i] * unsigned(index_caught[i]);
  }
  return _data[location];
}

template <typename T>
TensorTempl<T> &
TensorTempl<T>::operator+(const TensorTempl<T> & rhs)
{
  if (this->_shape != rhs._shape)
    mooseError("Improper shape for addition.");
  TensorTempl<T> * result = new TensorTempl<T>();
  result->_data.resize(this->_data.size());
  for (unsigned int i = 0; i < this->_data.size(); ++i)
    result->data[i] = this->_data[i] + rhs._data[i];
  result->_shape = this->_shape;
  result->_access_data = this->_access_data;
  return *result;
}

template <typename T>
TensorTempl<T> &
TensorTempl<T>::operator-(const TensorTempl<T> & rhs)
{
  if (this->_shape != rhs._shape)
    mooseError("Improper shape for addition.");
  TensorTempl<T> * result = new TensorTempl<T>();
  result->_data.resize(this->_data.size());
  for (unsigned int i = 0; i < this->_data.size(); ++i)
    result->data[i] = this->_data[i] - rhs._data[i];
  result->_shape = this->_shape;
  result->_access_data = this->_access_data;
  return *result;
}

template <typename T>
TensorTempl<T> & TensorTempl<T>::operator*(const TensorTempl<T> & rhs)
{
  return contractMult(*this, rhs, 1);
}

template <typename T>
TensorTempl<T> &
TensorTempl<T>::contractMult(const TensorTempl<T> & lhs,
                             const TensorTempl<T> & rhs,
                             unsigned int contractor)
{
  for (unsigned int i = 0; i < contractor; ++i)
    if (this->_shape[this->_shape.size() - 1 - i] != rhs._shape[i])
      mooseError("Improper shape for multiplication.");
  TensorTempl<T> * result = new TensorTempl<T>();
  result->_shape =
      std::vector<long unsigned int>(this->_shape.begin(), this->_shape.end() - contractor);
  result->_shape.insert(result->_shape.end(), rhs._shape.begin() + contractor, rhs._shape.end());
  unsigned int data_size = 1;
  for (unsigned int i = 0; i < result->_shape.size(); ++i)
    data_size *= result->_shape[i];

  std::vector<T> new_data(data_size, T(0.0));
  unsigned int divider = 1;
  for (unsigned int i = contractor; i < rhs._shape.size(); ++i)
    divider *= rhs._shape[i];

  std::vector<long unsigned int> center(this->_shape.end() - contractor, this->_shape.end());

  T begin;
  for (unsigned int i = 0; i < data_size; ++i)
  {
    begin = 0.0;
    recurseMult(
        begin, i, divider, center, 0, std::vector<unsigned int>(center.size(), 0), lhs, rhs);
    new_data[i] = begin;
  }
  result->_data = new_data;
  result->setAccessor();
  return *result;
}

template <typename T>
void
TensorTempl<T>::recurseMult(T & result_term,
                            unsigned int index,
                            unsigned int divider,
                            std::vector<long unsigned int> center,
                            unsigned int current,
                            std::vector<unsigned int> current_vec,
                            const TensorTempl<T> & lhs,
                            const TensorTempl<T> & rhs)
{
  for (unsigned int i = 0; i < current_vec.size(); ++i)
    if (current < center.size())
    {
      for (unsigned int i = 0; i < center[current]; ++i)
      {
        current_vec[current] = i;
        recurseMult(result_term, index, divider, center, current + 1, current_vec, lhs, rhs);
      }
      return;
    }
  unsigned int lhs_index = 0;
  unsigned int multiplier = 1;
  unsigned int rhs_index = index % divider;
  for (unsigned int i = 1; i <= center.size(); ++i)
  {
    lhs_index += multiplier * current_vec[center.size() - i];
    rhs_index += multiplier * current_vec[center.size() - i] * divider;
    multiplier *= center[center.size() - 1];
  }
  lhs_index += multiplier * (index / divider);
  result_term += lhs._data[lhs_index] * rhs._data[rhs_index];
}

template <typename T>
TensorTempl<T> & TensorTempl<T>::operator*(const T & rhs)
{
  TensorTempl<T> * result = new TensorTempl<T>();
  result->_shape = this->_shape;
  result->_access_data = this->_access_data;
  result->_data.resize(this->_data.size());
  for (unsigned int i = 0; i < result->_data.size(); ++i)
    result->data[i] = this->_data[i] * rhs;
  return *result;
}

template <typename T>
void
TensorTempl<T>::transpose()
{
  transpose(0, 1);
}

template <typename T>
void
TensorTempl<T>::transpose(unsigned int left_index, unsigned int right_index)
{
  std::iter_swap(_access_data.begin() + left_index, _access_data.begin() + right_index);
}

template <typename T>
void
TensorTempl<T>::setAccessor()
{
  _access_data = std::vector<unsigned int>(_shape.size(), 0);
  unsigned int multiplier = 1;
  for (int i = _shape.size() - 2; i >= 0; i--)
  {
    multiplier *= _shape[i + 1];
    _access_data[i] = multiplier;
  }
}

template <typename T>
template <typename U>
void
TensorTempl<T>::initialize(std::initializer_list<U> u, std::size_t _depth)
{
  // outer recursions
  checkDimensions(u.size(), _depth);
  for (auto t : u)
    initialize(t, _depth + 1);
}

template <typename T>
void
TensorTempl<T>::initialize(std::initializer_list<T> t, std::size_t _depth)
{
  // innermost recursion
  checkDimensions(t.size(), _depth);
  for (const T & v : t)
    _data.push_back(v);
  setAccessor();
}

template <typename T>
void
TensorTempl<T>::checkDimensions(std::size_t size, std::size_t _depth)
{
  // check size
  if (_shape.size() <= _depth)
  {
    _shape.resize(_depth + 1);
    _shape[_depth] = size;
  }
  else
  {
    if (_shape[_depth] != size)
      throw std::runtime_error("Inconsistent initializer list size");
  }
}

template <typename T>
void
TensorTempl<T>::applyFunction(void (*function)(T & t))
{
  for (unsigned int i = 0; i < _data.size(); ++i)
    function(_data[i]);
}
