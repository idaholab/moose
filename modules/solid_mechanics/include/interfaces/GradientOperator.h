//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "libmesh/vector_value.h"
#include "RankTwoTensor.h"

template <class C>
class GradientOperator : public C
{
protected:
  /// Returns the gradient operator in a given direction
  RankTwoTensor
  gradOp(unsigned int component, const RealVectorValue & g, const Real & v, const Point & p)
  {
    RankTwoTensor G;
    this->addGradOp(G, component, g, v, p);
    return G;
  }
};

namespace GradientOperatorCoordinates
{
class Cartesian
{
protected:
  /// Accumulate the contribution of the gradient operator in a given direction for a given tensor
  void addGradOp(RankTwoTensor & G,
                 unsigned int component,
                 const RealVectorValue & g,
                 const Real & /*v*/,
                 const Point & /*p*/)
  {
    for (auto j : make_range(3))
      G(component, j) += g(j);
  }
};

class AxisymmetricCylindrical
{
protected:
  /// Accumulate the contribution of the gradient operator in a given direction for a given tensor
  void addGradOp(RankTwoTensor & G,
                 unsigned int component,
                 const RealVectorValue & g,
                 const Real & v,
                 const Point & p)
  {
    for (auto j : make_range(2))
      G(component, j) += g(j);

    // R
    if (component == 0)
      G(2, 2) += v / p(0);
  }
};

class CentrosymmetricSpherical
{
protected:
  /// Accumulate the contribution of the gradient operator in a given direction for a given tensor
  void addGradOp(RankTwoTensor & G,
                 unsigned int /*component*/,
                 const RealVectorValue & g,
                 const Real & v,
                 const Point & p)
  {
    G(0, 0) += g(0);
    G(1, 1) += v / p(0);
    G(2, 2) += v / p(0);
  }
};
}

typedef GradientOperator<GradientOperatorCoordinates::Cartesian> GradientOperatorCartesian;
typedef GradientOperator<GradientOperatorCoordinates::AxisymmetricCylindrical>
    GradientOperatorAxisymmetricCylindrical;
typedef GradientOperator<GradientOperatorCoordinates::CentrosymmetricSpherical>
    GradientOperatorCentrosymmetricSpherical;
