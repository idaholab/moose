// $Id: quadrature_gauss.C 3874 2010-07-02 21:57:26Z roystgnr $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2008 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#include "ArbitraryQuadrature.h"


ArbitraryQuadrature::ArbitraryQuadrature(const unsigned int d,
                                         const Order o)
  : QBase(d,o)
{}


ArbitraryQuadrature::~ArbitraryQuadrature()
{}

void
ArbitraryQuadrature::setPoints(const std::vector<Point> & points)
{
  _points = points;
  _weights.resize(points.size(), 1.0);
}

void
ArbitraryQuadrature::init_1D(const ElemType _type,
                             unsigned int p_level)
{
}

void
ArbitraryQuadrature::init_2D(const ElemType _type,
                             unsigned int p_level)
{
}

void
ArbitraryQuadrature::init_3D(const ElemType _type,
                             unsigned int p_level)
{
}

