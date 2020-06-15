//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArbitraryQuadrature.h"

// libMesh includes
#include "libmesh/enum_quadrature_type.h"

ArbitraryQuadrature::ArbitraryQuadrature(const unsigned int d, const Order o) : QBase(d, o) {}

QuadratureType
ArbitraryQuadrature::type() const
{
  return INVALID_Q_RULE;
}

void
ArbitraryQuadrature::setPoints(const std::vector<Point> & points)
{
  _points = points;
  _weights.resize(points.size(), 1.0);
}

void
ArbitraryQuadrature::setWeights(const std::vector<Real> & weights)
{
  _weights = weights;
}

void
ArbitraryQuadrature::init_1D(const ElemType _type, unsigned int p_level)
{
  this->_type = _type;
  this->_p_level = p_level;
}

void
ArbitraryQuadrature::init_2D(const ElemType _type, unsigned int p_level)
{
  this->_type = _type;
  this->_p_level = p_level;
}

void
ArbitraryQuadrature::init_3D(const ElemType _type, unsigned int p_level)
{
  this->_type = _type;
  this->_p_level = p_level;
}
