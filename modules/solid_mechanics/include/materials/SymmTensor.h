/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SYMMTENSOR_H
#define SYMMTENSOR_H

#include "ColumnMajorMatrix.h"
#include "MaterialProperty.h"
#include "DataIO.h"
#include "MooseRandom.h"

#include <cmath>
#include "libmesh/libmesh.h"
#include "libmesh/point.h"

class SymmTensor
{
public:
  SymmTensor() : _xx(0), _yy(0), _zz(0), _xy(0), _yz(0), _zx(0) {}
  explicit SymmTensor(Real init) : _xx(init), _yy(init), _zz(init), _xy(init), _yz(init), _zx(init)
  {
  }
  SymmTensor(Real xx, Real yy, Real zz, Real xy, Real yz, Real zx)
    : _xx(xx), _yy(yy), _zz(zz), _xy(xy), _yz(yz), _zx(zx)
  {
  }
  explicit SymmTensor(const ColumnMajorMatrix & cmm)
    : _xx(cmm.numEntries() == 9 ? cmm.rawData()[0] : 0),
      _yy(cmm.numEntries() == 9 ? cmm.rawData()[4] : 0),
      _zz(cmm.numEntries() == 9 ? cmm.rawData()[8] : 0),
      _xy(cmm.numEntries() == 9 ? cmm.rawData()[1] : 0),
      _yz(cmm.numEntries() == 9 ? cmm.rawData()[5] : 0),
      _zx(cmm.numEntries() == 9 ? cmm.rawData()[2] : 0)
  {
    if (cmm.numEntries() != 9)
    {
      mooseError("Cannot create SymmTensor from ColumnMajorMatrix.  Wrong number of entries.");
    }
  }

  explicit SymmTensor(const std::vector<Real> & init_list)
    : _xx(init_list[0]),
      _yy(init_list[1]),
      _zz(init_list[2]),
      _xy(init_list[3]),
      _yz(init_list[4]),
      _zx(init_list[5])
  {
    // test the length to make sure it's 6 long
    if (init_list.size() != 6)
    {
      mooseError("SymmTensor initialization error: please enter a vector with 6 entries.");
    }
  }

  void fillFromInputVector(const std::vector<Real> & input)
  {
    if (input.size() != 6)
    {
      mooseError("SymmTensor error.  Input vector must have six entries.");
    }
    _xx = input[0];
    _yy = input[1];
    _zz = input[2];
    _xy = input[3];
    _yz = input[4];
    _zx = input[5];
  }

  Real rowDot(const unsigned int r, const libMesh::TypeVector<Real> & v) const
  {
    mooseAssert(LIBMESH_DIM == 3, "Incompatible sizes");
    if (0 == r)
    {
      return _xx * v(0) + _xy * v(1) + _zx * v(2);
    }
    else if (1 == r)
    {
      return _xy * v(0) + _yy * v(1) + _yz * v(2);
    }
    else if (2 == r)
    {
      return _zx * v(0) + _yz * v(1) + _zz * v(2);
    }
    else
    {
      mooseError("Incorrect row");
    }
    return 0;
  }

  Real trace() const { return _xx + _yy + _zz; }

  Real component(unsigned int i) const
  {
    if (0 == i)
    {
      return _xx;
    }
    else if (1 == i)
    {
      return _yy;
    }
    else if (2 == i)
    {
      return _zz;
    }
    else if (3 == i)
    {
      return _xy;
    }
    else if (4 == i)
    {
      return _yz;
    }
    else if (5 == i)
    {
      return _zx;
    }
    else
    {
      mooseError("Invalid entry requested for SymmTensor");
    }
    return 0;
  }
  Real xx() const { return _xx; }
  Real yy() const { return _yy; }
  Real zz() const { return _zz; }
  Real xy() const { return _xy; }
  Real yz() const { return _yz; }
  Real zx() const { return _zx; }
  Real yx() const { return _xy; }
  Real zy() const { return _yz; }
  Real xz() const { return _zx; }
  Real & xx() { return _xx; }
  Real & yy() { return _yy; }
  Real & zz() { return _zz; }
  Real & xy() { return _xy; }
  Real & yz() { return _yz; }
  Real & zx() { return _zx; }
  Real & yx() { return _xy; }
  Real & zy() { return _yz; }
  Real & xz() { return _zx; }
  Real & operator()(const unsigned i, const unsigned j)
  {
    Real * rVal(NULL);
    if (0 == i)
    {
      if (0 == j)
      {
        rVal = &_xx;
      }
      else if (1 == j)
      {
        rVal = &_xy;
      }
      else if (2 == j)
      {
        rVal = &_zx;
      }
    }
    else if (1 == i)
    {
      if (0 == j)
      {
        rVal = &_xy;
      }
      else if (1 == j)
      {
        rVal = &_yy;
      }
      else if (2 == j)
      {
        rVal = &_yz;
      }
    }
    else if (2 == i)
    {
      if (0 == j)
      {
        rVal = &_zx;
      }
      else if (1 == j)
      {
        rVal = &_yz;
      }
      else if (2 == j)
      {
        rVal = &_zz;
      }
    }
    if (!rVal)
    {
      mooseError("Index must be 0, 1, or 2");
    }
    return *rVal;
  }

  Real operator()(const unsigned i, const unsigned j) const
  {
    const Real * rVal(NULL);
    if (0 == i)
    {
      if (0 == j)
      {
        rVal = &_xx;
      }
      else if (1 == j)
      {
        rVal = &_xy;
      }
      else if (2 == j)
      {
        rVal = &_zx;
      }
    }
    else if (1 == i)
    {
      if (0 == j)
      {
        rVal = &_xy;
      }
      else if (1 == j)
      {
        rVal = &_yy;
      }
      else if (2 == j)
      {
        rVal = &_yz;
      }
    }
    else if (2 == i)
    {
      if (0 == j)
      {
        rVal = &_zx;
      }
      else if (1 == j)
      {
        rVal = &_yz;
      }
      else if (2 == j)
      {
        rVal = &_zz;
      }
    }
    if (!rVal)
    {
      mooseError("Index must be 0, 1, or 2");
    }
    return *rVal;
  }

  Real doubleContraction(const SymmTensor & rhs) const
  {
    return _xx * rhs._xx + _yy * rhs._yy + _zz * rhs._zz +
           2 * (_xy * rhs._xy + _yz * rhs._yz + _zx * rhs._zx);
  }

  void xx(Real xx) { _xx = xx; }
  void yy(Real yy) { _yy = yy; }
  void zz(Real zz) { _zz = zz; }
  void xy(Real xy) { _xy = xy; }
  void yz(Real yz) { _yz = yz; }
  void zx(Real zx) { _zx = zx; }
  void yx(Real yx) { _xy = yx; }
  void zy(Real zy) { _yz = zy; }
  void xz(Real xz) { _zx = xz; }

  void zero() { _xx = _yy = _zz = _xy = _yz = _zx = 0; }
  void identity()
  {
    _xx = _yy = _zz = 1;
    _xy = _yz = _zx = 0;
  }
  void addDiag(Real value)
  {
    _xx += value;
    _yy += value;
    _zz += value;
  }
  bool operator==(const SymmTensor & rhs) const
  {
    return _xx == rhs._xx && _yy == rhs._yy && _zz == rhs._zz && _xy == rhs._xy && _yz == rhs._yz &&
           _zx == rhs._zx;
  }
  bool operator!=(const SymmTensor & rhs) const { return !operator==(rhs); }

  SymmTensor & operator+=(const SymmTensor & t)
  {
    _xx += t._xx;
    _yy += t._yy;
    _zz += t._zz;
    _xy += t._xy;
    _yz += t._yz;
    _zx += t._zx;
    return *this;
  }

  SymmTensor & operator-=(const SymmTensor & t)
  {
    _xx -= t._xx;
    _yy -= t._yy;
    _zz -= t._zz;
    _xy -= t._xy;
    _yz -= t._yz;
    _zx -= t._zx;
    return *this;
  }

  SymmTensor operator+(const SymmTensor & t) const
  {
    SymmTensor r_val;
    r_val._xx = _xx + t._xx;
    r_val._yy = _yy + t._yy;
    r_val._zz = _zz + t._zz;
    r_val._xy = _xy + t._xy;
    r_val._yz = _yz + t._yz;
    r_val._zx = _zx + t._zx;
    return r_val;
  }

  SymmTensor operator*(Real t) const
  {
    SymmTensor r_val;

    r_val._xx = _xx * t;
    r_val._yy = _yy * t;
    r_val._zz = _zz * t;
    r_val._xy = _xy * t;
    r_val._yz = _yz * t;
    r_val._zx = _zx * t;
    return r_val;
  }

  Point operator*(const Point & p) const
  {
    return Point(_xx * p(0) + _xy * p(1) + _zx * p(2),
                 _xy * p(0) + _yy * p(1) + _yz * p(2),
                 _zx * p(0) + _yz * p(1) + _zz * p(2));
  }

  SymmTensor operator-(const SymmTensor & t) const
  {
    SymmTensor r_val;
    r_val._xx = _xx - t._xx;
    r_val._yy = _yy - t._yy;
    r_val._zz = _zz - t._zz;
    r_val._xy = _xy - t._xy;
    r_val._yz = _yz - t._yz;
    r_val._zx = _zx - t._zx;
    return r_val;
  }

  SymmTensor & operator+=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot add ColumnMajorMatrix to SymmTensor.  Wrong number of entries.");
    const Real * data = cmm.rawData();
    _xx += data[0];
    _xy += data[1];
    _zx += data[2];
    _yy += data[4];
    _yz += data[5];
    _zz += data[8];
    return *this;
  }

  SymmTensor & operator-=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot add ColumnMajorMatrix to SymmTensor.  Wrong number of entries.");
    const Real * data = cmm.rawData();

    _xx -= data[0];
    _xy -= data[1];
    _zx -= data[2];
    _yy -= data[4];
    _yz -= data[5];
    _zz -= data[8];
    return *this;
  }

  SymmTensor & operator=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot set SymmTensor to ColumnMajorMatrix.  Wrong number of entries.");
    const Real * data = cmm.rawData();
    _xx = data[0];
    _xy = data[1];
    _zx = data[2];
    _yy = data[4];
    _yz = data[5];
    _zz = data[8];
    return *this;
  }

  SymmTensor & operator=(Real val)
  {
    _xx = val;
    _xy = val;
    _zx = val;
    _yy = val;
    _yz = val;
    _zz = val;
    return *this;
  }

  SymmTensor & operator*=(Real val)
  {
    _xx *= val;
    _xy *= val;
    _zx *= val;
    _yy *= val;
    _yz *= val;
    _zz *= val;
    return *this;
  }

  ColumnMajorMatrix columnMajorMatrix() const
  {
    ColumnMajorMatrix cmm(3, 3);
    cmm(0, 0) = _xx;
    cmm(1, 0) = _xy;
    cmm(2, 0) = _zx;
    cmm(0, 1) = _xy;
    cmm(1, 1) = _yy;
    cmm(2, 1) = _yz;
    cmm(0, 2) = _zx;
    cmm(1, 2) = _yz;
    cmm(2, 2) = _zz;
    return cmm;
  }

  friend std::ostream & operator<<(std::ostream & stream, const SymmTensor & obj);

  static void initRandom()
  {

    unsigned int randinit = 2000;
    MooseRandom::seed(randinit);
  }

  static SymmTensor genRandomSymmTensor(Real scalefactor)
  {

    SymmTensor tensor;

    tensor.xx() = (MooseRandom::rand() + 1.0) * scalefactor;
    tensor.yy() = (MooseRandom::rand() + 1.0) * scalefactor;
    tensor.zz() = (MooseRandom::rand() + 1.0) * scalefactor;
    tensor.xy() = (MooseRandom::rand() + 1.0) * scalefactor;
    tensor.yz() = (MooseRandom::rand() + 1.0) * scalefactor;
    tensor.zx() = (MooseRandom::rand() + 1.0) * scalefactor;

    return tensor;
  }

private:
  Real _xx;
  Real _yy;
  Real _zz;
  Real _xy;
  Real _yz;
  Real _zx;
};

template <>
PropertyValue * MaterialProperty<SymmTensor>::init(int size);

template <>
void dataStore(std::ostream & stream, const SymmTensor & v, void * /*context*/);

template <>
void dataLoad(std::istream & stream, SymmTensor & v, void * /*context*/);

#endif // SYMMTENSOR_H
