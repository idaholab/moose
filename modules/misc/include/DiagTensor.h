/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DIAGTENSOR_H
#define DIAGTENSOR_H

#include "ColumnMajorMatrix.h"
#include "MaterialProperty.h"
#include <cmath>
#include "libmesh/libmesh.h"

class DiagTensor
{
public:
  DiagTensor() : _xx(0), _yy(0), _zz(0) {}
  explicit

      DiagTensor(Real init)
    : _xx(init), _yy(init), _zz(init)
  {
  }
  DiagTensor(Real xx, Real yy, Real zz) : _xx(xx), _yy(yy), _zz(zz) {}
  explicit

      DiagTensor(const ColumnMajorMatrix & cmm)
    : _xx(cmm.numEntries() == 9 ? cmm.rawData()[0] : 0),
      _yy(cmm.numEntries() == 9 ? cmm.rawData()[4] : 0),
      _zz(cmm.numEntries() == 9 ? cmm.rawData()[8] : 0)
  {
    if (cmm.numEntries() != 9)
    {
      mooseError("Cannot create DiagTensor from ColumnMajorMatrix.  Wrong number of entries.");
    }
  }
  explicit

      DiagTensor(std::vector<Real> & init_list)
    : _xx(init_list[0]), _yy(init_list[1]), _zz(init_list[2])

  {
    // test the length to make sure it's 3 long
    if (init_list.size() != 3)
      mooseError("please enter a vector with 3 entries.");
  }

  Real rowDot(const unsigned int r, const libMesh::TypeVector<Real> & v) const
  {
    mooseAssert(LIBMESH_DIM == 3, "Incompatible sizes");
    if (0 == r)
    {
      return _xx * v(0);
    }
    else if (1 == r)
    {
      return _yy * v(1);
    }
    else if (2 == r)
    {
      return _zz * v(2);
    }
    else
    {
      mooseError("Incorrect row");
    }
    return 0;
  }

  libMesh::TypeVector<Real> operator*(const libMesh::TypeVector<Real> & v) const
  {
    libMesh::TypeVector<Real> r_val(v);

    r_val(0) = _xx * v(0);
    r_val(1) = _yy * v(1);
    r_val(2) = _zz * v(2);
    return r_val;
  }

  libMesh::VectorValue<Real> operator*(const libMesh::VectorValue<Real> & v) const
  {
    libMesh::VectorValue<Real> r_val(v);

    r_val(0) = _xx * v(0);
    r_val(1) = _yy * v(1);
    r_val(2) = _zz * v(2);
    return r_val;
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
    else
    {
      mooseError("Invalid entry requested for DiagTensor");
    }
    return 0;
  }
  Real xx() const { return _xx; }
  Real yy() const { return _yy; }
  Real zz() const { return _zz; }
  Real & operator()(const unsigned i, const unsigned j)
  {
    Real * rVal(NULL);
    if (0 == i)
    {
      if (0 == j)
      {
        rVal = &_xx;
      }
    }
    else if (1 == i)
    {
      if (1 == j)
      {
        rVal = &_yy;
      }
    }
    else if (2 == i)
    {
      if (2 == j)
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

  Real doubleContraction(const DiagTensor & rhs) const
  {
    return _xx * rhs._xx + _yy * rhs._yy + _zz * rhs._zz;
  }

  void assign(unsigned int i, Real val)
  {
    if (0 == i)
    {
      _xx = val;
    }
    else if (1 == i)
    {
      _yy = val;
    }
    else if (2 == i)
    {
      _zz = val;
    }
    else
    {
      mooseError("Invalid entry requested for DiagTensor");
    }
  }

  void xyz(Real xx, Real yy, Real zz)
  {
    _xx = xx;
    _yy = yy;
    _zz = zz;
  }
  void xx(Real xx) { _xx = xx; }
  void yy(Real yy) { _yy = yy; }
  void zz(Real zz) { _zz = zz; }

  void zero() { _xx = _yy = _zz = 0; }
  void identity() { _xx = _yy = _zz = 1.0; }
  void addDiag(Real value)
  {
    _xx += value;
    _yy += value;
    _zz += value;
  }
  bool operator==(const DiagTensor & rhs) const
  {
    return _xx == rhs._xx && _yy == rhs._yy && _zz == rhs._zz;
  }
  bool operator!=(const DiagTensor & rhs) const { return !operator==(rhs); }

  DiagTensor & operator+=(const DiagTensor & t)
  {
    _xx += t._xx;
    _yy += t._yy;
    _zz += t._zz;
    return *this;
  }

  DiagTensor & operator-=(const DiagTensor & t)
  {
    _xx -= t._xx;
    _yy -= t._yy;
    _zz -= t._zz;
    return *this;
  }

  DiagTensor operator+(const DiagTensor & t) const
  {
    DiagTensor r_val;
    r_val._xx = _xx + t._xx;
    r_val._yy = _yy + t._yy;
    r_val._zz = _zz + t._zz;
    return r_val;
  }

  DiagTensor inverse() const
  {
    DiagTensor r_val;
    mooseAssert((_xx == 0 || _yy == 0 || _zz == 0), "Cannot invert singular DiagTensor.");
    r_val._xx = 1. / _xx;
    r_val._yy = 1. / _yy;
    r_val._zz = 1. / _zz;
    return r_val;
  }

  DiagTensor sinvert() const
  {
    DiagTensor r_val;
    r_val._xx = 0;
    r_val._yy = 0;
    r_val._zz = 0;

    Real _eps = 1e-15;

    if (_xx != 0)
      r_val._xx = 1. / (_xx + _eps);
    if (_yy != 0)
      r_val._yy = 1. / (_yy + _eps);
    if (_zz != 0)
      r_val._zz = 1. / (_zz + _eps);

    return r_val;
  }

  DiagTensor operator*(Real t) const
  {
    DiagTensor r_val;

    r_val._xx = _xx * t;
    r_val._yy = _yy * t;
    r_val._zz = _zz * t;
    return r_val;
  }

  DiagTensor operator*(DiagTensor & t) const
  {
    DiagTensor r_val;

    r_val._xx = _xx * t._xx;
    r_val._yy = _yy * t._yy;
    r_val._zz = _zz * t._zz;
    return r_val;
  }

  DiagTensor operator-(const DiagTensor & t) const
  {
    DiagTensor r_val;
    r_val._xx = _xx - t._xx;
    r_val._yy = _yy - t._yy;
    r_val._zz = _zz - t._zz;
    return r_val;
  }

  DiagTensor & operator+=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot add ColumnMajorMatrix to DiagTensor.  Wrong number of entries.");
    const Real * data = cmm.rawData();
    _xx += data[0];
    _yy += data[4];
    _zz += data[8];
    return *this;
  }

  DiagTensor & operator-=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot add ColumnMajorMatrix to DiagTensor.  Wrong number of entries.");
    const Real * data = cmm.rawData();
    _xx -= data[0];
    _yy -= data[4];
    _zz -= data[8];
    return *this;
  }

  DiagTensor & operator=(const ColumnMajorMatrix & cmm)
  {
    mooseAssert(cmm.numEntries() == 9,
                "Cannot set DiagTensor to ColumnMajorMatrix.  Wrong number of entries.");
    const Real * data = cmm.rawData();
    _xx = data[0];
    _yy = data[4];
    _zz = data[8];
    return *this;
  }

  DiagTensor & operator=(Real val)
  {
    _xx = val;
    _yy = val;
    _zz = val;
    return *this;
  }

  DiagTensor & operator*=(Real val)
  {
    _xx *= val;
    _yy *= val;
    _zz *= val;
    return *this;
  }

  ColumnMajorMatrix columnMajorMatrix() const
  {
    ColumnMajorMatrix cmm(3, 3);
    cmm(0, 0) = _xx;
    cmm(1, 0) = 0;
    cmm(2, 0) = 0;
    cmm(0, 1) = 0;
    cmm(1, 1) = _yy;
    cmm(2, 1) = 0;
    cmm(0, 2) = 0;
    cmm(1, 2) = 0;
    cmm(2, 2) = _zz;
    return cmm;
  }

  friend std::ostream & operator<<(std::ostream & stream, const DiagTensor & obj);

  /**
   * Rotate the strain around the c-axis
   */
  void rotate(const Real a)
  {
    Real angle = a * pi / 180.0;
    Real s = std::sin(angle);
    Real c = std::cos(angle);

    _xx = _xx * c * c + _yy * s * s;
    _yy = _xx * s * s + _yy * c * c;
  }

  void fillFromInputVector(std::vector<Real> input)
  {
    if (input.size() != 3)
      mooseError("Please check the number of entries in the eigenstrain input vector");
    _xx = input[0];
    _yy = input[1];
    _zz = input[2];
  }

private:
  Real _xx;
  Real _yy;
  Real _zz;
};

template <>
PropertyValue * MaterialProperty<DiagTensor>::init(int size);

#endif // DIAGTENSOR_H
