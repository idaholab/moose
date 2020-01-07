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

template <typename T, typename S>
class MooseADWrapperBase
{
public:
  typedef S DNType;

  MooseADWrapperBase() : _val() {}
  MooseADWrapperBase(MooseADWrapperBase<T, S> &&) = default;
  virtual ~MooseADWrapperBase() = default;

  /**
   * Returns the value for any case where a MaterialProperty is requested as a regular (non-AD)
   * property
   */
  const T & value() const
  {
    synchronizeToVal();
    return _val;
  }

  /**
   * Returns the value for any case where a MaterialProperty is declared as a regular (non-AD)
   * property (used only during material calculations)
   */
  T & value()
  {
    synchronizeToVal();
    _dual_is_dirty = true;
    return _val;
  }

  MooseADWrapperBase<T, S> & operator=(const MooseADWrapperBase<T, S> & rhs)
  {
    _val = rhs._val;
    if (_dual_number && rhs._dual_number)
      *_dual_number = *rhs._dual_number;
    else if (_dual_number)
      copyValueToDualNumber();
    return *this;
  }

  MooseADWrapperBase<T, S> & operator=(MooseADWrapperBase<T, S> &&) = default;

  /**
   * Returns the dual number for any case where a MaterialProperty is requested as a AD property
   */
  virtual const DNType & dn(bool = true) const
  {
    synchronizeToDual();
    return *_dual_number;
  }

  /**
   * Returns the value for any case where a MaterialProperty is declared as an AD property (used
   * only during material calculations)
   */
  virtual DNType & dn(bool = true)
  {
    synchronizeToDual();
    _val_is_dirty = true;
    return *_dual_number;
  }

  void synchronizeToVal() const
  {
    mooseAssert(!(_dual_is_dirty && _val_is_dirty),
                "dual/non-dual material property synchronization is broken");
    if (!_val_is_dirty)
      return;

    if (!_dual_number)
      initializeDual();
    if (_dual_number)
      copyDualNumberToValue();
    _val_is_dirty = false;
  }

  void synchronizeToDual() const
  {
    mooseAssert(!(_dual_is_dirty && _val_is_dirty),
                "dual/non-dual material property synchronization is broken");
    if (!_dual_is_dirty)
      return;

    if (!_dual_number)
      initializeDual();
    if (_dual_number)
      copyValueToDualNumber();
    _dual_is_dirty = false;
  }

protected:
  virtual void initializeDual() const = 0;
  virtual void copyDualNumberToValue() const = 0;
  virtual void copyValueToDualNumber() const = 0;

  mutable T _val;
  mutable std::unique_ptr<DNType> _dual_number = nullptr;

private:
  mutable bool _dual_is_dirty = true;
  mutable bool _val_is_dirty = false;
};

template <typename T>
class MooseADWrapper : public MooseADWrapperBase<T, T>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<T> &&) = default;
  MooseADWrapper<T> & operator=(const MooseADWrapper<T> &) = default;
  virtual ~MooseADWrapper() = default;

  virtual const T & dn(bool requested_by_user = true) const override
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return MooseADWrapperBase<T, T>::dn();
  }

  /**
   * Returns the value for any case where a MaterialProperty is declared as an AD property (used
   * only during material calculations)
   */
  virtual T & dn(bool requested_by_user = true) override
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return MooseADWrapperBase<T, T>::dn();
  }

protected:
  virtual void copyDualNumberToValue() const override { this->_val = *(this->_dual_number); }
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }
  virtual void initializeDual() const override {}

private:
  friend void dataStore<T>(std::ostream &, MooseADWrapper<T> &, void *);
  friend void dataLoad<T>(std::istream &, MooseADWrapper<T> &, void *);
};

template <>
class MooseADWrapper<Real> : public MooseADWrapperBase<Real, DualReal>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<Real> &&) = default;
  MooseADWrapper<Real> & operator=(const MooseADWrapper<Real> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }

private:
  friend void dataStore<Real>(std::ostream &, MooseADWrapper<Real> &, void *);
  friend void dataLoad<Real>(std::istream &, MooseADWrapper<Real> &, void *);
};

template <>
class MooseADWrapper<VectorValue<Real>>
  : public MooseADWrapperBase<VectorValue<Real>, VectorValue<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<VectorValue<Real>> &&) = default;
  MooseADWrapper<VectorValue<Real>> &
  operator=(const MooseADWrapper<VectorValue<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }

private:
  friend void
  dataStore<VectorValue<Real>>(std::ostream &, MooseADWrapper<VectorValue<Real>> &, void *);
  friend void
  dataLoad<VectorValue<Real>>(std::istream &, MooseADWrapper<VectorValue<Real>> &, void *);
};

template <>
class MooseADWrapper<TensorValue<Real>>
  : public MooseADWrapperBase<TensorValue<Real>, TensorValue<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<TensorValue<Real>> &&) = default;
  MooseADWrapper<TensorValue<Real>> &
  operator=(const MooseADWrapper<TensorValue<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }

private:
  friend void
  dataStore<TensorValue<Real>>(std::ostream &, MooseADWrapper<TensorValue<Real>> &, void *);
  friend void
  dataLoad<TensorValue<Real>>(std::istream &, MooseADWrapper<TensorValue<Real>> &, void *);
};

template <>
class MooseADWrapper<RankTwoTensorTempl<Real>>
  : public MooseADWrapperBase<RankTwoTensorTempl<Real>, RankTwoTensorTempl<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<RankTwoTensorTempl<Real>> &&) = default;
  MooseADWrapper<RankTwoTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankTwoTensorTempl<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }

private:
  friend void dataStore<RankTwoTensorTempl<Real>>(std::ostream &,
                                                  MooseADWrapper<RankTwoTensorTempl<Real>> &,
                                                  void *);
  friend void dataLoad<RankTwoTensorTempl<Real>>(std::istream &,
                                                 MooseADWrapper<RankTwoTensorTempl<Real>> &,
                                                 void *);
};

template <>
class MooseADWrapper<RankThreeTensorTempl<Real>>
  : public MooseADWrapperBase<RankThreeTensorTempl<Real>, RankThreeTensorTempl<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<RankThreeTensorTempl<Real>> &&) = default;
  MooseADWrapper<RankThreeTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankThreeTensorTempl<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override
  {
    *(this->_dual_number) = RankThreeTensorTempl<DualReal>(this->_val);
  }

private:
  friend void dataStore<RankThreeTensorTempl<Real>>(std::ostream &,
                                                    MooseADWrapper<RankThreeTensorTempl<Real>> &,
                                                    void *);
  friend void dataLoad<RankThreeTensorTempl<Real>>(std::istream &,
                                                   MooseADWrapper<RankThreeTensorTempl<Real>> &,
                                                   void *);
};

template <>
class MooseADWrapper<RankFourTensorTempl<Real>>
  : public MooseADWrapperBase<RankFourTensorTempl<Real>, RankFourTensorTempl<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<RankFourTensorTempl<Real>> &&) = default;
  MooseADWrapper<RankFourTensorTempl<Real>> &
  operator=(const MooseADWrapper<RankFourTensorTempl<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override { *(this->_dual_number) = this->_val; }

private:
  friend void dataStore<RankFourTensorTempl<Real>>(std::ostream &,
                                                   MooseADWrapper<RankFourTensorTempl<Real>> &,
                                                   void *);
  friend void dataLoad<RankFourTensorTempl<Real>>(std::istream &,
                                                  MooseADWrapper<RankFourTensorTempl<Real>> &,
                                                  void *);
};

template <>
class MooseADWrapper<DenseVector<Real>>
  : public MooseADWrapperBase<DenseVector<Real>, DenseVector<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<DenseVector<Real>> &&) = default;
  MooseADWrapper<DenseVector<Real>> &
  operator=(const MooseADWrapper<DenseVector<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override;

private:
  friend void
  dataStore<DenseVector<Real>>(std::ostream &, MooseADWrapper<DenseVector<Real>> &, void *);
  friend void
  dataLoad<DenseVector<Real>>(std::istream &, MooseADWrapper<DenseVector<Real>> &, void *);
};

template <>
class MooseADWrapper<DenseMatrix<Real>>
  : public MooseADWrapperBase<DenseMatrix<Real>, DenseMatrix<DualReal>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<DenseMatrix<Real>> &&) = default;
  MooseADWrapper<DenseMatrix<Real>> &
  operator=(const MooseADWrapper<DenseMatrix<Real>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override;

private:
  friend void
  dataStore<DenseMatrix<Real>>(std::ostream &, MooseADWrapper<DenseMatrix<Real>> &, void *);
  friend void
  dataLoad<DenseMatrix<Real>>(std::istream &, MooseADWrapper<DenseMatrix<Real>> &, void *);
};

template <>
class MooseADWrapper<std::vector<DenseVector<Real>>>
  : public MooseADWrapperBase<std::vector<DenseVector<Real>>, std::vector<DenseVector<DualReal>>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<std::vector<DenseVector<Real>>> &&) = default;
  MooseADWrapper<std::vector<DenseVector<Real>>> &
  operator=(const MooseADWrapper<std::vector<DenseVector<Real>>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override;

private:
  friend void dataStore<std::vector<DenseVector<Real>>>(
      std::ostream &, MooseADWrapper<std::vector<DenseVector<Real>>> &, void *);
  friend void dataLoad<std::vector<DenseVector<Real>>>(
      std::istream &, MooseADWrapper<std::vector<DenseVector<Real>>> &, void *);
};

template <>
class MooseADWrapper<std::vector<DenseMatrix<Real>>>
  : public MooseADWrapperBase<std::vector<DenseMatrix<Real>>, std::vector<DenseMatrix<DualReal>>>
{
public:
  MooseADWrapper() = default;
  MooseADWrapper(MooseADWrapper<std::vector<DenseMatrix<Real>>> &&) = default;
  MooseADWrapper<std::vector<DenseMatrix<Real>>> &
  operator=(const MooseADWrapper<std::vector<DenseMatrix<Real>>> &) = default;
  virtual ~MooseADWrapper() = default;

protected:
  virtual void initializeDual() const override;
  virtual void copyDualNumberToValue() const override;
  virtual void copyValueToDualNumber() const override;

private:
  friend void dataStore<std::vector<DenseMatrix<Real>>>(
      std::ostream &, MooseADWrapper<std::vector<DenseMatrix<Real>>> &, void *);
  friend void dataLoad<std::vector<DenseMatrix<Real>>>(
      std::istream &, MooseADWrapper<std::vector<DenseMatrix<Real>>> &, void *);
};
