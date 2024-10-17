//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"

#include "libmesh/quadrature.h"

/**
 * Implements a fake quadrature rule where you can specify the locations
 * (in the reference domain) of the quadrature points.
 */
class ArbitraryQuadrature : public libMesh::QBase
{
public:
  ArbitraryQuadrature(const unsigned int _dim,
                      const libMesh::Order _order = libMesh::INVALID_ORDER);

  /**
   * Copy/move ctor, copy/move assignment operator, and destructor are
   * all explicitly defaulted for this simple class.
   */
  ArbitraryQuadrature(const ArbitraryQuadrature &) = default;
  ArbitraryQuadrature(ArbitraryQuadrature &&) = default;
  ArbitraryQuadrature & operator=(const ArbitraryQuadrature &) = default;
  ArbitraryQuadrature & operator=(ArbitraryQuadrature &&) = default;
  virtual ~ArbitraryQuadrature() = default;

  libMesh::QuadratureType type() const override;

  /**
   * Set the quadrature points. Note that this also sets the quadrature weights to unity
   */
  void setPoints(const std::vector<libMesh::Point> & points);

  /**
   * Set the quadrature weights
   */
  void setWeights(const std::vector<libMesh::Real> & weights);

  virtual bool shapes_need_reinit() override { return true; }

  virtual std::unique_ptr<libMesh::QBase> clone() const override;

private:
  /**
   * These functions must be defined to fulfill the interface expected
   * by the quadrature initialization routines.  Please do not
   * modify the function names or signatures.
   */
  void init_1D(const libMesh::ElemType _type = libMesh::INVALID_ELEM,
               unsigned int p_level = 0) override;
  void init_2D(const libMesh::ElemType _type = libMesh::INVALID_ELEM,
               unsigned int p_level = 0) override;
  void init_3D(const libMesh::ElemType _type = libMesh::INVALID_ELEM,
               unsigned int p_level = 0) override;
};
