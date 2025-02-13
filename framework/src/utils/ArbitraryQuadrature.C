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

using namespace libMesh;

ArbitraryQuadrature::ArbitraryQuadrature(const unsigned int d, const Order o) : QBase(d, o) {}

std::unique_ptr<QBase>
ArbitraryQuadrature::clone() const
{
  return std::make_unique<ArbitraryQuadrature>(*this);
}

QuadratureType
ArbitraryQuadrature::type() const
{
  return libMesh::INVALID_Q_RULE;
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

#ifdef LIBMESH_QBASE_INIT_ARGUMENTS_REMOVED
void
ArbitraryQuadrature::init_1D()
{
}

void
ArbitraryQuadrature::init_2D()
{
}

void
ArbitraryQuadrature::init_3D()
{
}

#else
void
ArbitraryQuadrature::init_1D(const ElemType, unsigned int)
{
}

void
ArbitraryQuadrature::init_2D(const ElemType, unsigned int)
{
}

void
ArbitraryQuadrature::init_3D(const ElemType, unsigned int)
{
}
#endif // LIBMESH_QBASE_INIT_ARGUMENTS_REMOVED
