/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ArbitraryQuadrature.h"


ArbitraryQuadrature::ArbitraryQuadrature(const unsigned int d, const Order o) :
    QBase(d,o)
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
  this->_type = _type;
  this->_p_level = p_level;
}

void
ArbitraryQuadrature::init_2D(const ElemType _type,
                             unsigned int p_level)
{
  this->_type = _type;
  this->_p_level = p_level;
}

void
ArbitraryQuadrature::init_3D(const ElemType _type,
                             unsigned int p_level)
{
  this->_type = _type;
  this->_p_level = p_level;
}
