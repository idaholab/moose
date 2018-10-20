#ifndef MOOSEADWRAPPER_H
#define MOOSEADWRAPPER_H

#include "MooseTypes.h"
#include "MooseError.h"

#include "libmesh/dense_matrix.h"

template <typename T>
class MooseADWrapper
{
public:
  MooseADWrapper() : _val() {}

  typedef T DNType;

  const T & value() const { return _val; }

  T & value() { return _val; }

  const T & dn() const
  {
    mooseError("You need to template this class if you want it to work with automatic "
               "differentiation and dual numbers.");
    return _val;
  }

  T & dn()
  {
    mooseError("You need to template this class if you want it to work with automatic "
               "differentiation and dual numbers.");
    return _val;
  }

private:
  T _val;
};

template <template <typename> class W>
class MooseADWrapper<W<Real>>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef W<ADReal> DNType;

  const W<Real> & value() const { return _val; }

  W<Real> & value() { return _val; }

  const W<ADReal> & dn() const { return _dual_number; }

  W<ADReal> & dn() { return _dual_number; }

private:
  W<Real> _val;
  W<ADReal> _dual_number;
};

template <>
class MooseADWrapper<Real>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef ADReal DNType;

  const Real & value() const { return _val; }

  Real & value() { return _val; }

  const ADReal & dn() const { return _dual_number; }

  ADReal & dn() { return _dual_number; }

private:
  Real _val;
  ADReal _dual_number;
};

template <>
class MooseADWrapper<libMesh::DenseMatrix<Real>>
{
public:
  MooseADWrapper() : _val() {}

  typedef libMesh::DenseMatrix<Real> DNType;

  const libMesh::DenseMatrix<Real> & value() const { return _val; }

  libMesh::DenseMatrix<Real> & value() { return _val; }

  const libMesh::DenseMatrix<Real> & dn() const
  {
    mooseError("DenseMatrix does not currently support automatic differentiation.");
    return _val;
  }

  libMesh::DenseMatrix<Real> & dn()
  {
    mooseError("DenseMatrix does not currently support automatic differentiation.");
    return _val;
  }

private:
  libMesh::DenseMatrix<Real> _val;
};

#endif // MOOSEADWRAPPER_H
