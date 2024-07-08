//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransfiniteMeshGenerator.h"
#include "CastUniquePointer.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"

#include "libmesh/parsed_function.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/fparser_ad.hh"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", TransfiniteMeshGenerator);

InputParameters
TransfiniteMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  MooseEnum edge_type("LINE=1 CIRCARC=2 DISCRETE=3 PARSED=4");

  params.addRequiredParam<std::vector<Point>>("corners", "The x,y,z positions of the nodes");

  // Define edge types
  params.addRequiredParam<MooseEnum>("bottom_type", edge_type, "type of the bottom (y) boundary");
  params.addRequiredParam<MooseEnum>("top_type", edge_type, "type of the top (y) boundary");
  params.addRequiredParam<MooseEnum>("left_type", edge_type, "type of the left (x) boundary");
  params.addRequiredParam<MooseEnum>("right_type", edge_type, "type of the right (x) boundary");

  // We need to know the number of points on opposite sides, and if no other parameter is available
  // we shall assume them to be equally distributed
  params.addRequiredParam<unsigned int>("nx",
                                        "Number of nodes on horizontal edges, including corners");
  params.addRequiredParam<unsigned int>("ny",
                                        "Number of Nodes on vertical edges, including corners");

  // each edge has a different parameter according to its type
  params.addParam<std::string>("bottom_parameter", "", "Bottom side support parameter");
  params.addParam<std::string>("top_parameter", "", "Top side support parameter");
  params.addParam<std::string>("left_parameter", "", "Left side support parameter");
  params.addParam<std::string>("right_parameter", "", "Right side support parameter");

  params.addRangeCheckedParam<Real>(
      "bias_x",
      1.,
      "bias_x>=1.0 & bias_x<=2",
      "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>(
      "bias_y",
      1.,
      "bias_y>=1.0 & bias_y<=2",
      "The amount by which to grow (or shrink) the cells in the y-direction.");

  params.addClassDescription(
      "Creates a QUAD4 mesh given a set of corner vertices and edge types. "
      "The edge type can be either LINE, CIRCARC, DISCRETE or PARSED, with "
      "LINE as the default option. "
      "For the non-default options the user needs to specify additional "
      "parameters via the edge_parameter option "
      "as follows: for CIRCARC the deviation of the midpoint from an "
      "arccircle, for DISCRETE a set of points, or "
      "a paramterization via the PARSED option. Opposite edges may have "
      "different distributions s long as the "
      "number of points is identical. Along opposite edges a different point "
      "distribution can be prescribed "
      "via the options bias_x or bias_y for opposing edges.");

  params.addParamNamesToGroup("bottom_type left_type top_type right_type", "Edge type");
  params.addParamNamesToGroup("bottom_parameter left_parameter top_parameter right_parameter",
                              "Edge");
  params.addParamNamesToGroup("nx ny bias_x bias_y", "Number and distribution of points");

  return params;
}

TransfiniteMeshGenerator::TransfiniteMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _corners(getParam<std::vector<Point>>("corners")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _bottom_type(getParam<MooseEnum>("bottom_type")),
    _top_type(getParam<MooseEnum>("top_type")),
    _left_type(getParam<MooseEnum>("left_type")),
    _right_type(getParam<MooseEnum>("right_type")),
    _bottom_parameter(getParam<std::string>("bottom_parameter")),
    _top_parameter(getParam<std::string>("top_parameter")),
    _left_parameter(getParam<std::string>("left_parameter")),
    _right_parameter(getParam<std::string>("right_parameter")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y"))
{
  // initialize parsed function
  _parsed_func = std::make_shared<SymFunction>();
  setParserFeatureFlags(_parsed_func);
  _parsed_func->AddConstant("pi", libMesh::pi);
  _func_params.resize(1);

  mooseAssert((_nx > 1) && (_ny > 1),
              "A minimum of 2 points is needed on each edge, i.e. the user needs to consider edge "
              "vertices as well.");
}
std::unique_ptr<MeshBase>
TransfiniteMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // explicitly assign corners since they will be used extensively and the ordering may be confusing
  const Point V00 = _corners[0];
  const Point V10 = _corners[1];
  const Point V11 = _corners[2];
  const Point V01 = _corners[3];

  // we construct a vector that mimics the normal only to account for inward and outward arcircles
  // note we do not need to define inward directions since they would multiply the sign of the
  // arc circle parameter, which if negative would give inward vectors
  std::vector<Point> outward_vec(4);
  outward_vec[0] = Point(0.0, -1.0, 0.0);
  outward_vec[1] = Point(0.0, 1.0, 0.0);
  outward_vec[2] = Point(-1.0, 0.0, 0.0);
  outward_vec[3] = Point(1.0, 0.0, 0.0);

  const unsigned long int total_nodes = _nx * _ny;

  // we take [0,1] as the reference interval and we need to set the biases upfront
  Real edge_length = 1.0;
  std::vector<Real> param_x_dir = getPointsDistribution(edge_length, _nx, _bias_x);
  std::vector<Real> param_y_dir = getPointsDistribution(edge_length, _ny, _bias_y);

  std::vector<Point> edge_bottom =
      getEdge(V00, V10, _nx, _bottom_type, _bottom_parameter, outward_vec[0], param_x_dir);
  std::vector<Point> edge_top =
      getEdge(V01, V11, _nx, _top_type, _top_parameter, outward_vec[1], param_x_dir);
  std::vector<Point> edge_left =
      getEdge(V00, V01, _ny, _left_type, _left_parameter, outward_vec[2], param_y_dir);
  std::vector<Point> edge_right =
      getEdge(V10, V11, _ny, _right_type, _right_parameter, outward_vec[3], param_y_dir);

  // Used for the parametrization on edge pairs, provided by the point distribution according to
  // biases
  Real rx_coord, sy_coord;

  std::vector<Node *> nodes(total_nodes);
  unsigned int node_id = 0;

  Point newPt;
  Real r1_basis, r2_basis, s1_basis, s2_basis;

  for (unsigned int idx = 0; idx < _nx; idx++)
  {
    rx_coord = param_x_dir[idx];
    r1_basis = 1 - rx_coord;
    r2_basis = rx_coord;

    for (unsigned int idy = 0; idy < _ny; idy++)
    {
      sy_coord = param_y_dir[idy];
      s1_basis = 1 - sy_coord;
      s2_basis = sy_coord;

      // this is the core of the algorithm and generates every internal point
      newPt = r2_basis * edge_right[idy] + r1_basis * edge_left[idy] + s1_basis * edge_bottom[idx] +
              s2_basis * edge_top[idx] - r1_basis * s1_basis * V00 - r1_basis * s2_basis * V01 -
              r2_basis * s1_basis * V10 - r2_basis * s2_basis * V11;

      nodes[node_id] = mesh->add_point(newPt, node_id);
      node_id++;
    }
  }

  for (unsigned int idx = 0; idx < _nx - 1; idx++)
  {
    for (unsigned int idy = 0; idy < _ny - 1; idy++)
    {
      Elem * elem = mesh->add_elem(new Quad4);
      elem->set_node(0) = nodes[idy + idx * _ny];
      elem->set_node(1) = nodes[idy + (idx + 1) * _ny];
      elem->set_node(2) = nodes[idy + 1 + (idx + 1) * _ny];
      elem->set_node(3) = nodes[idy + 1 + idx * _ny];

      if (idy == 0) // add bottom boundary (boundary_id = 0)
        boundary_info.add_side(elem, 0, 0);

      if (idx == _nx - 2) // add right boundary (boundary_id = 1)
        boundary_info.add_side(elem, 1, 1);

      if (idy == _ny - 2) // add top boundary (boundary_id = 2)
        boundary_info.add_side(elem, 2, 2);

      if (idx == 0) // add left boundary (boundary_id = 3)
        boundary_info.add_side(elem, 3, 3);
    }
  }

  boundary_info.sideset_name(0) = "bottom";
  boundary_info.nodeset_name(0) = "bottom";

  boundary_info.sideset_name(1) = "right";
  boundary_info.nodeset_name(1) = "right";

  boundary_info.sideset_name(2) = "top";
  boundary_info.nodeset_name(2) = "top";

  boundary_info.sideset_name(3) = "left";
  boundary_info.nodeset_name(3) = "left";

  mesh->prepare_for_use();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::vector<Point>
TransfiniteMeshGenerator::getEdge(const Point & P1,
                                  const Point & P2,
                                  const unsigned int np,
                                  const MooseEnum & type,
                                  const std::string & parameter,
                                  const Point & outward,
                                  const std::vector<Real> & param_vec)
{
  std::vector<Point> edge;

  switch (type)
  {
    case 1:
    {
      edge = getLineEdge(P1, P2, param_vec);
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[0] - P1).norm(), 0.0),
                  "The line does not fit the first vertex on the edge.");
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[np - 1] - P2).norm(), 0.0),
                  "The line does not fit the end vertex on the edge.");
    }
    break;
    case 2:
    {
      edge = getCircarcEdge(P1, P2, parameter, outward, param_vec);
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[0] - P1).norm(), 0.0),
                  "No arccircle parametrization can be found to fit the first vertex on the edge.");
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[np - 1] - P2).norm(), 0.0),
                  "No arccircle parametrization can be found to fit the end vertex on the edge.");
    }
    break;
    case 3:
    {
      edge = getDiscreteEdge(np, parameter);
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[0] - P1).norm(), 0.0),
                  "The first discrete point does not fit the corresponding edge vertex."
                  "Note: discrete points need to replicate the edge corners.");
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[np - 1] - P2).norm(), 0.0),
                  "The last discrete point does not fit the corresponding edge vertex."
                  "Note: discrete points need to replicate the edge corners.");
    }
    break;
    case 4:
    {
      edge = getParsedEdge(parameter, param_vec);
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[0] - P1).norm(), 0.0),
                  "The parametrization does not fit the first vertex on the edge.");
      mooseAssert(MooseUtils::absoluteFuzzyEqual((edge[np - 1] - P2).norm(), 0.0),
                  "The parametrization does not fit the end vertex on the edge.");
    }
    break;
  }
  if (edge.size() != np)
    mooseError("The generated edge does not match the number of points on the"
               "opposite edge.");
  return edge;
}

std::vector<Point>
TransfiniteMeshGenerator::getLineEdge(const Point & P1,
                                      const Point & P2,
                                      const std::vector<Real> & param_vec)
{
  std::vector<Point> edge(param_vec.size());
  auto it = 0;

  for (auto rx : param_vec)
  {
    edge[it] = P1 * (1.0 - rx) + P2 * rx;
    it++;
  };
  return edge;
}

std::vector<Point>
TransfiniteMeshGenerator::getParsedEdge(const std::string & parameter,
                                        const std::vector<Real> & param_vec)
{
  std::vector<Point> edge(param_vec.size());
  Real x_coord, y_coord;

  std::vector<std::string> param_coords;
  MooseUtils::tokenize(parameter, param_coords, 1, ";");

  auto it = 0;
  for (auto rx : param_vec)
  {
    _parsed_func->Parse(param_coords[0], "r");
    x_coord = _parsed_func->Eval(&rx);
    _parsed_func->Parse(param_coords[1], "r");
    y_coord = _parsed_func->Eval(&rx);
    edge[it] = Point(x_coord, y_coord, 0.0);
    it++;
  }

  return edge;
}

std::vector<Point>
TransfiniteMeshGenerator::getDiscreteEdge(const unsigned int np, const std::string & parameter)
{
  std::vector<Point> edge(np);
  std::vector<std::string> string_points;
  MooseUtils::tokenize(parameter, string_points, 1, "\n");
  if (string_points.size() != np)
    mooseError("DISCRETE: the number of discrete points does not match the number of points on the"
               "opposite edge.");

  auto it = 0;
  for (unsigned int iter = 0; iter < string_points.size(); iter++)
  {
    std::vector<Real> point_vals;
    MooseUtils::tokenizeAndConvert(string_points[iter], point_vals, " ");
    edge[it] = Point(point_vals[0], point_vals[1], point_vals[2]);
    it++;
  }

  return edge;
}

std::vector<Point>
TransfiniteMeshGenerator::getCircarcEdge(const Point & P1,
                                         const Point & P2,
                                         const std::string & parameter,
                                         const Point & outward,
                                         const std::vector<Real> & param_vec)
{
  std::vector<Point> edge(param_vec.size());

  std::vector<Real> param_coords;
  MooseUtils::tokenizeAndConvert(parameter, param_coords, ";");
  Point P3;
  if (param_coords.size() == 1)
  {
    P3 = computeMidPoint(P1, P2, param_coords[0], outward);
  }
  else
  {
    P3 = Point(param_coords[0], param_coords[1], param_coords[2]);
  }

  Real rad = computeRadius(P1, P2, P3);
  Point P0 = computeOrigin(P1, P2, P3);

  // need to shift to center of coordinates to find the corresponding radians
  Point x0 = (P1 - P0);
  Point x1 = (P2 - P0);

  // The case when the edge spans quadrants 1 and 4 requires special treatment
  // to periodically switch we compute the angle that needs added to one edge
  // to identify the entire edge span
  mooseAssert(x0.norm() > 0.0 && x1.norm() > 0.0,
              "The point provided cannot generate an arc circle on the edge specified");

  Real arclength = std::acos((x0 * x1) / x0.norm() / x1.norm());

  Real a = std::atan2(x0(1), x0(0));
  Real b = std::atan2(x1(1), x1(0));

  if (a < 0)
    a = a + 2 * M_PI;
  if (std::abs(b - a) > M_PI)
    b = a + arclength;

  auto it = 0;
  for (auto rx : param_vec)
  {
    Real interval = getMapInterval(rx, 0.0, 1.0, a, b);
    Real x = P0(0) + rad * std::cos(interval);
    Real y = P0(1) + rad * std::sin(interval);
    edge[it] = Point(x, y, 0.0);
    it++;
  };

  return edge;
}

Real
TransfiniteMeshGenerator::getMapInterval(
    const Real xab, const Real a, const Real b, const Real c, const Real d) const
// this routine maps a point x\in[a, b] to the corresponding point in the interval [c, d]
{
  mooseAssert(std::abs(b - a) > 0.0,
              "The input interval [a, b] is empty, check that a and b are not identical.");
  Real xcd = c + (d - c) / (b - a) * (xab - a);

  return xcd;
}

std::vector<Real>
TransfiniteMeshGenerator::getPointsDistribution(const Real edge_length,
                                                const unsigned int np,
                                                const Real bias) const
{
  std::vector<Real> param_vec;
  Real rx = 0.0;

  if (bias != 1.0)
  {
    Real step = 0.0;
    Real factor = edge_length * (1.0 - std::abs(bias)) / (1.0 - std::pow(std::abs(bias), np - 1));
    param_vec.push_back(rx);
    for (unsigned int iter = 1; iter < np; iter++)
    {
      step = step + factor * std::pow(bias, Real(iter - 1));
      rx = getMapInterval(step, 0.0, edge_length, 0.0, 1.0);
      param_vec.push_back(rx);
    }
  }
  else
  {
    const Real interval = edge_length / Real(np - 1);
    for (unsigned int iter = 0; iter < np; iter++)
    {
      rx = getMapInterval(Real(iter) * interval, 0.0, edge_length, 0.0, 1.0);
      param_vec.push_back(rx);
    }
  }
  return param_vec;
}

Real
TransfiniteMeshGenerator::computeRadius(const Point & P1, const Point & P2, const Point & P3) const
{
  Point temp1 = P1 - P2;
  Real a2 = temp1.norm_sq(); // a is the distance from P1 to P2, but we only need it squared
  Point temp2 = P3 - P1;
  Real b2 = temp2.norm_sq();
  Real dr = std::sqrt(b2 - a2 / 4.0);
  const Real rad = a2 / 8.0 / dr + dr / 2.0;
  return rad;
}

Point
TransfiniteMeshGenerator::computeOrigin(const Point & P1, const Point & P2, const Point & P3) const
{
  // define interim quantities
  const Real a1 = (2 * P3(0) - 2 * P1(0));
  const Real a2 = (2 * P3(1) - 2 * P1(1));
  const Real A = P3(1) * P3(1) - P1(0) * P1(0) + P3(0) * P3(0) - P1(1) * P1(1);
  const Real b1 = (2 * P3(0) - 2 * P2(0));
  const Real b2 = (2 * P3(1) - 2 * P2(1));
  const Real B = P3(1) * P3(1) - P2(0) * P2(0) + P3(0) * P3(0) - P2(1) * P2(1);
  mooseAssert(std::abs(b2) > 0.0 && std::abs(a1) > 0.0 && std::abs(a1 * b2 - a2 * b1) > 0.0,
              "The point provided cannot generate an arc circle on the edge specified."
              "The origin of the corresponding circle cannot be computed.");

  const Real y0 = (a1 * B - A * b1) / (a1 * b2 - a2 * b1);
  const Real x0 = (A - y0 * a2) / a1;

  // fill in and return the origin point
  Point P0 = Point(x0, y0, 0.0);
  return P0;
}

Point
TransfiniteMeshGenerator::computeMidPoint(const Point & P1,
                                          const Point & P2,
                                          const Real height,
                                          const Point & outward) const
{
  const Real xm = (P1(0) + P2(0)) / 2;
  const Real ym = (P1(1) + P2(1)) / 2;
  // The arc can be inverted into the domain (concave) or outward (convex)
  // we use the convention that if the height is given as negative we seek a concave arc
  // if "height" is positive then we seek a convex arc.
  // The outward vector is sufficient to determine the direction of the arc.
  const Real dist = std::abs(height);
  const int orient = (height >= 0) ? 1 : -1;
  Point MidPoint;

  // this accounts for lines aligned with the system of coordinates
  if ((std::abs(P2(0) - P1(0)) < 1e-15) || (std::abs(P2(1) - P1(1)) < 1e-15))
    MidPoint = Point(xm, ym, 0.0) + dist * orient * outward;

  // some of the cases are  not covered by this strategy and we need to use normals info
  // this is implemented and tested in the prototype and is soon to be updated
  if (std::abs(P2(0) - P1(0)) > 1e-15 && std::abs(P2(1) - P1(1)) > 1e-15)
  {
    // m is the slope of the line orthogonal to the midpoint
    const Real m = -(P2(0) - P1(0)) / (P2(1) - P1(1));
    // The equation to determine allows two solutions
    const Real x_temp = dist * sqrt((m * m + 1)) / (m * m + 1);
    const Real factor = orient * outward(0) + m * orient * outward(1);
    int direction = (factor >= 0) ? 1 : -1;

    MidPoint = Point(direction * x_temp + xm, direction * x_temp * m + ym, 0.0);
  }
  return MidPoint;
}
