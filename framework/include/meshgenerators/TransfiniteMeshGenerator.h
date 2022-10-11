
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
  const Real _bias_x;
  const Real _bias_y;

  // We allow different types
  const MooseEnum _left_type;
  const MooseEnum _right_type;
  const MooseEnum _top_type;
  const MooseEnum _bottom_type;

  // So far the intention is to read in paramters a strings and
  // typecast them after parsing and checking the edge type
  const std::string _left_parameter;
  const std::string _top_parameter;
  const std::string _bottom_parameter;
  const std::string _right_parameter;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _parsed_func;

  // This is the main routine for constructing edges according to the user input
  std::vector<Point> getEdge(const Point & P1,
                             const Point & P2,
                             const unsigned int & np,
                             const MooseEnum & type,
                             const std::string & parameter,
                             const Point & outward,
                             const Real & bias);

  // The following 3 routines are needed for generating arc circles given the user input
  // The input is expected to be the distance from a stright line at the middle of an edge
  Real computeRadius(const Point & P1,
                    const Point & P2,
                    const Point & P3) const;
  Point computeOrigin(const Point & P1,
                    const Point & P2,
                    const Point & P3) const;
  Point computeMidPoint(const Point & P1,
                    const Point & P2,
                    const Real & dist,
                    const Point & outward) const;

  std::vector<Point> getParsedEdge(const Point & P1,
                    const Point & P2,
                    const unsigned int & np,
                    const std::string & parameter,
                    const Real & bias);

  // The following routines are necessary for the paramterization of opposite edges
  // To assure we have the same parameterization on opposite edges we need to map it to
  //  a reference interval, i.e. [0, 1]
  Real getMapToReference(const Real & x,
                        const Real & a,
                        const Real & b) const;
  Real getMapFromReference(const Real & x,
                        const Real & a,
                        const Real & b) const;
  // For a circle the paramterization is based on radians and we need to compute
  //  the angles spanned between 2 end vertices
  Real getPolarAngle(const Point & P) const;
  Real getEdgeLength(const Point & P1,
                     const Point & P2) const;
  std::vector<Real> getParametrization(const Real & edge_length,
                          const unsigned int & np,
                          const Real & bias) const;

  usingFunctionParserUtilsMembers(false);

};
