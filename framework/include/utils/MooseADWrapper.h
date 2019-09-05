//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "DualReal.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

#include "libmesh/dense_vector.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/dense_matrix.h"

#include "DualRealOps.h"

#include <typeinfo>

template <typename T>
class MooseADWrapper;

template <typename T>
void dataStore(std::ostream & stream, MooseADWrapper<T> & dn_wrapper, void * context);
template <typename T>
void dataLoad(std::istream & stream, MooseADWrapper<T> & dn_wrapper, void * context);

template <typename T>
class MooseADWrapper
{
public:
  MooseADWrapper(bool = false) : _val(), _dual_number(nullptr) {}
  MooseADWrapper(MooseADWrapper<T> &&) = default;

  MooseADWrapper<T> & operator=(const MooseADWrapper<T> & rhs)
  {
    _val = rhs._val;
    return *this;
  }
  MooseADWrapper<T> & operator=(MooseADWrapper<T> &&) = default;

  typedef T DNType;

  /**
   * Returns the value for any case where a MaterialProperty is requested as a regular (non-AD)
   * property
   */
  const T & value() const { return _val; }

  /**
   * Returns the value for any case where a MaterialProperty is declared as a regular (non-AD)
   * property (used only during material calculations)
   */
  T & value() { return _val; }

  /**
   * Returns the dual number for any case where a MaterialProperty is requested as a AD property
   */
  const T & dn(bool requested_by_user = true) const
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return _val;
  }

  /**
   * Returns the value for any case where a MaterialProperty is declared as an AD property (used
   * only during material calculations)
   */
  T & dn(bool requested_by_user = true)
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return _val;
  }

  /**
   * Used during Jacobian calculations in case a property was declared as AD but a consuming object
   * has requested it as a regular property
   */
  void copyDualNumberToValue() {}

  /**
   * Mark a change in whether to use AD or not. This is relevant during stateful material
   * calculations. E.g. when a current property becomes old we call markAD(false) and when we swap
   * an old material property to the current property (see MaterialPropertyStorage::shift) we call
   * markAD(true)
   */
  void markAD(bool /*use_ad*/) {}

private:
  T _val;
  mutable std::unique_ptr<T> _dual_number;
  friend void dataStore<T>(std::ostream &, MooseADWrapper<T> &, void *);
  friend void dataLoad<T>(std::istream &, MooseADWrapper<T> &, void *);
};

template <>
class MooseADWrapper<Real>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<Real> &&) = default;

  typedef DualReal DNType;

  const Real & value() const { return _val; }

  Real & value() { return _val; }

  const DualReal & dn(bool = true) const;

  DualReal & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<Real> & operator=(const MooseADWrapper<Real> &);
  MooseADWrapper<Real> & operator=(MooseADWrapper<Real> &&) = default;

private:
  bool _use_ad;
  Real _val;
  mutable std::unique_ptr<DualReal> _dual_number;
  friend void dataStore<Real>(std::ostream &, MooseADWrapper<Real> &, void *);
  friend void dataLoad<Real>(std::istream &, MooseADWrapper<Real> &, void *);
};

template <>
class MooseADWrapper<VectorValue<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<VectorValue<Real>> &&) = default;

  typedef VectorValue<DualReal> DNType;

  const VectorValue<Real> & value() const { return _val; }

  VectorValue<Real> & value() { return _val; }

  const VectorValue<DualReal> & dn(bool = true) const;

  VectorValue<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<VectorValue<Real>> & operator=(const MooseADWrapper<VectorValue<Real>> &);
  MooseADWrapper<VectorValue<Real>> & operator=(MooseADWrapper<VectorValue<Real>> &&) = default;

private:
  bool _use_ad;
  VectorValue<Real> _val;
  mutable std::unique_ptr<VectorValue<DualReal>> _dual_number;
  friend void
  dataStore<VectorValue<Real>>(std::ostream &, MooseADWrapper<VectorValue<Real>> &, void *);
  friend void
  dataLoad<VectorValue<Real>>(std::istream &, MooseADWrapper<VectorValue<Real>> &, void *);
};

template <>
class MooseADWrapper<TensorValue<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<TensorValue<Real>> &&) = default;

  typedef TensorValue<DualReal> DNType;

  const TensorValue<Real> & value() const { return _val; }

  TensorValue<Real> & value() { return _val; }

  const TensorValue<DualReal> & dn(bool = true) const;

  TensorValue<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<TensorValue<Real>> & operator=(const MooseADWrapper<TensorValue<Real>> &);
  MooseADWrapper<TensorValue<Real>> & operator=(MooseADWrapper<TensorValue<Real>> &&) = default;

private:
  bool _use_ad;
  TensorValue<Real> _val;
  mutable std::unique_ptr<TensorValue<DualReal>> _dual_number;
  friend void
  dataStore<TensorValue<Real>>(std::ostream &, MooseADWrapper<TensorValue<Real>> &, void *);
  friend void
  dataLoad<TensorValue<Real>>(std::istream &, MooseADWrapper<TensorValue<Real>> &, void *);
};

template <>
class MooseADWrapper<RankTwoTensorTempl<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<RankTwoTensorTempl<Real>> &&) = default;

  typedef RankTwoTensorTempl<DualReal> DNType;

  const RankTwoTensorTempl<Real> & value() const { return _val; }

  RankTwoTensorTempl<Real> & value() { return _val; }

  const RankTwoTensorTempl<DualReal> & dn(bool = true) const;

  RankTwoTensorTempl<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<RankTwoTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankTwoTensorTempl<Real>> &);
  MooseADWrapper<RankTwoTensorTempl<Real>> &
  operator=(MooseADWrapper<RankTwoTensorTempl<Real>> &&) = default;

private:
  bool _use_ad;
  RankTwoTensorTempl<Real> _val;
  mutable std::unique_ptr<RankTwoTensorTempl<DualReal>> _dual_number;
  friend void dataStore<RankTwoTensorTempl<Real>>(std::ostream &,
                                                  MooseADWrapper<RankTwoTensorTempl<Real>> &,
                                                  void *);
  friend void dataLoad<RankTwoTensorTempl<Real>>(std::istream &,
                                                 MooseADWrapper<RankTwoTensorTempl<Real>> &,
                                                 void *);
};

template <>
class MooseADWrapper<RankThreeTensorTempl<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<RankThreeTensorTempl<Real>> &&) = default;

  typedef RankThreeTensorTempl<DualReal> DNType;

  const RankThreeTensorTempl<Real> & value() const { return _val; }

  RankThreeTensorTempl<Real> & value() { return _val; }

  const RankThreeTensorTempl<DualReal> & dn(bool = true) const;

  RankThreeTensorTempl<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<RankThreeTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankThreeTensorTempl<Real>> &);
  MooseADWrapper<RankThreeTensorTempl<Real>> &
  operator=(MooseADWrapper<RankThreeTensorTempl<Real>> &&) = default;

private:
  bool _use_ad;
  RankThreeTensorTempl<Real> _val;
  mutable std::unique_ptr<RankThreeTensorTempl<DualReal>> _dual_number;
  friend void dataStore<RankThreeTensorTempl<Real>>(std::ostream &,
                                                    MooseADWrapper<RankThreeTensorTempl<Real>> &,
                                                    void *);
  friend void dataLoad<RankThreeTensorTempl<Real>>(std::istream &,
                                                   MooseADWrapper<RankThreeTensorTempl<Real>> &,
                                                   void *);
};

template <>
class MooseADWrapper<RankFourTensorTempl<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<RankFourTensorTempl<Real>> &&) = default;

  typedef RankFourTensorTempl<DualReal> DNType;

  const RankFourTensorTempl<Real> & value() const { return _val; }

  RankFourTensorTempl<Real> & value() { return _val; }

  const RankFourTensorTempl<DualReal> & dn(bool = true) const;

  RankFourTensorTempl<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<RankFourTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankFourTensorTempl<Real>> &);
  MooseADWrapper<RankFourTensorTempl<Real>> &
  operator=(MooseADWrapper<RankFourTensorTempl<Real>> &&) = default;

private:
  bool _use_ad;
  RankFourTensorTempl<Real> _val;
  mutable std::unique_ptr<RankFourTensorTempl<DualReal>> _dual_number;
  friend void dataStore<RankFourTensorTempl<Real>>(std::ostream &,
                                                   MooseADWrapper<RankFourTensorTempl<Real>> &,
                                                   void *);
  friend void dataLoad<RankFourTensorTempl<Real>>(std::istream &,
                                                  MooseADWrapper<RankFourTensorTempl<Real>> &,
                                                  void *);
};

template <>
class MooseADWrapper<DenseVector<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<DenseVector<Real>> &&) = default;

  typedef DenseVector<DualReal> DNType;

  const DenseVector<Real> & value() const { return _val; }

  DenseVector<Real> & value() { return _val; }

  const DenseVector<DualReal> & dn(bool = true) const;

  DenseVector<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<DenseVector<Real>> & operator=(const MooseADWrapper<DenseVector<Real>> &);
  MooseADWrapper<DenseVector<Real>> & operator=(MooseADWrapper<DenseVector<Real>> &&) = default;

private:
  bool _use_ad;
  DenseVector<Real> _val;
  mutable std::unique_ptr<DenseVector<DualReal>> _dual_number;
  friend void
  dataStore<DenseVector<Real>>(std::ostream &, MooseADWrapper<DenseVector<Real>> &, void *);
  friend void
  dataLoad<DenseVector<Real>>(std::istream &, MooseADWrapper<DenseVector<Real>> &, void *);
};

template <>
class MooseADWrapper<DenseMatrix<Real>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<DenseMatrix<Real>> &&) = default;

  typedef DenseMatrix<DualReal> DNType;

  const DenseMatrix<Real> & value() const { return _val; }

  DenseMatrix<Real> & value() { return _val; }

  const DenseMatrix<DualReal> & dn(bool = true) const;

  DenseMatrix<DualReal> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<DenseMatrix<Real>> & operator=(const MooseADWrapper<DenseMatrix<Real>> &);
  MooseADWrapper<DenseMatrix<Real>> & operator=(MooseADWrapper<DenseMatrix<Real>> &&) = default;

private:
  bool _use_ad;
  DenseMatrix<Real> _val;
  mutable std::unique_ptr<DenseMatrix<DualReal>> _dual_number;
  friend void
  dataStore<DenseMatrix<Real>>(std::ostream &, MooseADWrapper<DenseMatrix<Real>> &, void *);
  friend void
  dataLoad<DenseMatrix<Real>>(std::istream &, MooseADWrapper<DenseMatrix<Real>> &, void *);
};

template <>
class MooseADWrapper<std::vector<DenseMatrix<Real>>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<std::vector<DenseMatrix<Real>>> &&) = default;

  typedef std::vector<DenseMatrix<DualReal>> DNType;

  const std::vector<DenseMatrix<Real>> & value() const { return _val; }

  std::vector<DenseMatrix<Real>> & value() { return _val; }

  const std::vector<DenseMatrix<DualReal>> & dn(bool = true) const;

  std::vector<DenseMatrix<DualReal>> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<std::vector<DenseMatrix<Real>>> &
  operator=(const MooseADWrapper<std::vector<DenseMatrix<Real>>> &);
  MooseADWrapper<std::vector<DenseMatrix<Real>>> &
  operator=(MooseADWrapper<std::vector<DenseMatrix<Real>>> &&) = default;

private:
  bool _use_ad;
  std::vector<DenseMatrix<Real>> _val;
  mutable std::unique_ptr<std::vector<DenseMatrix<DualReal>>> _dual_number;
  friend void dataStore<std::vector<DenseMatrix<Real>>>(
      std::ostream &, MooseADWrapper<std::vector<DenseMatrix<Real>>> &, void *);
  friend void dataLoad<std::vector<DenseMatrix<Real>>>(
      std::istream &, MooseADWrapper<std::vector<DenseMatrix<Real>>> &, void *);
};

template <>
class MooseADWrapper<std::vector<DenseVector<Real>>>
{
public:
  MooseADWrapper(bool use_ad = false);
  MooseADWrapper(MooseADWrapper<std::vector<DenseVector<Real>>> &&) = default;

  typedef std::vector<DenseVector<DualReal>> DNType;

  const std::vector<DenseVector<Real>> & value() const { return _val; }

  std::vector<DenseVector<Real>> & value() { return _val; }

  const std::vector<DenseVector<DualReal>> & dn(bool = true) const;

  std::vector<DenseVector<DualReal>> & dn(bool = true);

  void copyDualNumberToValue();

  void markAD(bool use_ad);

  MooseADWrapper<std::vector<DenseVector<Real>>> &
  operator=(const MooseADWrapper<std::vector<DenseVector<Real>>> &);
  MooseADWrapper<std::vector<DenseVector<Real>>> &
  operator=(MooseADWrapper<std::vector<DenseVector<Real>>> &&) = default;

private:
  bool _use_ad;
  std::vector<DenseVector<Real>> _val;
  mutable std::unique_ptr<std::vector<DenseVector<DualReal>>> _dual_number;
  friend void dataStore<std::vector<DenseVector<Real>>>(
      std::ostream &, MooseADWrapper<std::vector<DenseVector<Real>>> &, void *);
  friend void dataLoad<std::vector<DenseVector<Real>>>(
      std::istream &, MooseADWrapper<std::vector<DenseVector<Real>>> &, void *);
};
