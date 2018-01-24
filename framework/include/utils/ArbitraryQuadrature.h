//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ARBITRARYQUADRATURE_H
#define ARBITRARYQUADRATURE_H

// MOOSE includes
#include "Moose.h" // using namespace libMesh

#include "libmesh/quadrature.h"

/**
 * Implements a fake quadrature rule where you can specify the locations
 * (in the reference domain) of the quadrature points.
 */
class ArbitraryQuadrature : public QBase
{
public:
  ArbitraryQuadrature(const unsigned int _dim, const Order _order = INVALID_ORDER);

  virtual ~ArbitraryQuadrature() = default;

  QuadratureType type() const override;

  void setPoints(const std::vector<Point> & points);

  virtual bool shapes_need_reinit() override { return true; }

private:
  /**
   * These functions must be defined to fulfill the interface expected
   * by the quadrature initialization routines.  Please do not
   * modify the function names or signatures.
   */
  void init_1D(const ElemType _type = INVALID_ELEM, unsigned int p_level = 0) override;
  void init_2D(const ElemType _type = INVALID_ELEM, unsigned int p_level = 0) override;
  void init_3D(const ElemType _type = INVALID_ELEM, unsigned int p_level = 0) override;
};

#endif // ARBITRARYQUADRATURE_H
