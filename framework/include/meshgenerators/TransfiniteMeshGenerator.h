
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "FunctionParserUtils.h"
//#include "FunctionInterface.h"
#include "libmesh/parsed_function.h"
#include <vector>

/**
 * Generates an quadrilateral given all the parameters
 */
class TransfiniteMeshGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  TransfiniteMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  // The corners of the domain
  const std::vector<Point> & _corners;
  const unsigned int _nx;
  const unsigned int _ny;

  // We allow different types
  const MooseEnum _bottom_type;
  const MooseEnum _top_type;
  const MooseEnum _left_type;
  const MooseEnum _right_type;

  // So far the intention is to read in paramters a strings and
  // typecast them after parsing and checking the edge type
  const std::string _bottom_parameter;
  const std::string _top_parameter;
  const std::string _left_parameter;
  const std::string _right_parameter;

  const Real _bias_x;
  const Real _bias_y;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _parsed_func;

  // This is the main routine for constructing edges according to the user input
  std::vector<Point> getEdge(const Point & P1,
                             const Point & P2,
                             const unsigned int np,
                             const MooseEnum & type,
                             const std::string & parameter,
                             const Point & outward,
                             const std::vector<Real> & param_vec);

  std::vector<Point> getParsedEdge(const std::string & parameter,
                                   const std::vector<Real> & param_vec);

  std::vector<Point> getCircarcEdge(const Point & P1,
                                    const Point & P2,
                                    const std::string & parameter,
                                    const Point & outward,
                                    const std::vector<Real> & param_vec);

  std::vector<Point> getDiscreteEdge(const unsigned int np, const std::string & parameter);

  std::vector<Point>
  getLineEdge(const Point & P1, const Point & P2, const std::vector<Real> & param_vec);

  // The following 3 routines are needed for generating arc circles given the user input
  // The input is expected to be the distance from a stright line at the middle of an edge
  Real computeRadius(const Point & P1, const Point & P2, const Point & P3) const;

  // To identify the origin of a circle that passes through 3 points we need to solve a system of
  // 3 equations. The system is solved separately and this routine implements its solution.
  Point computeOrigin(const Point & P1, const Point & P2, const Point & P3) const;

  // Given two points and a distance from a straight line at the middle of the edge, we have two
  // possible solutions for the origin of the circle. This routine chooses the midpoint given
  // the orientation of the edge specified by the outward vector. The user provides the orientation
  // using a sign convention on the "dist" parameter, with positive for inward and negative for
  // outward. The outward vector is precalculated and used in the routine to choose the correct
  // midpoint.
  Point
  computeMidPoint(const Point & P1, const Point & P2, const Real dist, const Point & outward) const;

  // The following routine is necessary for the parametrization of opposite edges
  // To assure we have the same parameterization on opposite edges we need to map it to
  //  a reference interval, i.e. [0, 1], and refer back and forth as needed.
  // This routine maps a point x\in[a, b] to the corresponding point in the interval [c, d].

  Real getMapInterval(const Real xab, const Real a, const Real b, const Real c, const Real d) const;

  std::vector<Real>
  getPointsDistribution(const Real edge_length, const unsigned int np, const Real bias) const;

  usingFunctionParserUtilsMembers(false);
};
