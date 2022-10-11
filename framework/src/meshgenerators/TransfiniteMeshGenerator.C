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
  params.addRequiredParam<MooseEnum>("left", edge_type, "type of the left (x) boundary");
  params.addRequiredParam<MooseEnum>("right", edge_type, "type of the right (x) boundary");

  params.addRequiredParam<MooseEnum>("top", edge_type, "type of the top (y) boundary");
  params.addRequiredParam<MooseEnum>("bottom", edge_type, "type of the bottom (y) boundary");
  // We need to know the number of points on opposite sides, and if no other parameter is available
  // we shall assume them to be equally distributed
  params.addRequiredParam<unsigned int>("nx",
                                        "Number of nodes on horizontal edges, including corners");
  params.addRequiredParam<unsigned int>("ny",
                                        "Number of Nodes on vertical edges, including corners");

  // each edge has a different paramter according to its type
  params.addParam<std::string>("left_parameter", "", "Left side support parameter");
  params.addParam<std::string>("top_parameter", "", "Top side support parameter");
  params.addParam<std::string>("bottom_parameter", "", "Bottom side support parameter");
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

  //params.addParam<FunctionName>("top_function",
  //                            "Function expression encoding a paramterization of an edge");
  params.addClassDescription(
      "Creates a quad mesh given a set of corner vertices and edge types."
      "The edge type is assumed line by default, if a curvilinear edge is desired"
      "the user needs to provide either a set of points, via the DISCRETE option, or "
      "a paramterization via the PARSED option, arc circles can be computed with the flag CIRCARC."
      "Opposite edges may have different distributions of points prescribed via a bias as long as the"
      "number of points is identical.");

  return params;
}

TransfiniteMeshGenerator::TransfiniteMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _corners(getParam<std::vector<Point>>("corners")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _left_type(getParam<MooseEnum>(
        "left")), // the types will have asserts and defaults such as equidistant lines
    _right_type(getParam<MooseEnum>("right")),
    _top_type(getParam<MooseEnum>("top")),
    _bottom_type(getParam<MooseEnum>("bottom")),
    _left_parameter(getParam<std::string>(
        "left_parameter")), // so far we intend to allow different parameter types
    _top_parameter(getParam<std::string>("top_parameter")),
    _bottom_parameter(getParam<std::string>("bottom_parameter")),
    _right_parameter(getParam<std::string>("right_parameter")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y"))
{
   //initialize parsed function
    _parsed_func = std::make_shared<SymFunction>();
   setParserFeatureFlags(_parsed_func);
   _parsed_func->AddConstant("pi", libMesh::pi);
   _func_params.resize(1);

}
std::unique_ptr<MeshBase>
TransfiniteMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(2);
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // explicitly assign corners since they will be used extensively
  const Point V00 = _corners[0];
  const Point V10 = _corners[1];
  const Point V11 = _corners[2];
  const Point V01 = _corners[3];

  //we construct a vector that mimics the normal only to account for inward and outward arcircles
  //note we do not need to define inward directions since they would multiply the sign of the
  //arc circle parameter, which if negative would give inward vectors
  std::vector<Point > outward_vec(4);
  outward_vec[0]=Point( 0.0, -1.0, 0.0);
  outward_vec[1]=Point( 0.0,  1.0, 0.0);
  outward_vec[2]=Point(-1.0,  0.0, 0.0);
  outward_vec[3]=Point( 1.0,  0.0, 0.0);

  const unsigned total_nodes = _nx * _ny;


  std::vector<Point> edge_bottom; // parametrized via nx
  std::vector<Point> edge_top;    // parametrized via nx
  std::vector<Point> edge_left;   // parametrized via ny
  std::vector<Point> edge_right;  // parametrized via ny

  edge_bottom = getEdge(V00, V10, _nx, _bottom_type, _bottom_parameter, outward_vec[0], _bias_x);
  edge_top = getEdge(V01, V11, _nx, _top_type, _top_parameter,outward_vec[1], _bias_x);
  edge_left = getEdge(V00, V01, _ny, _left_type, _left_parameter,outward_vec[2], _bias_y);
  edge_right = getEdge(V10, V11, _ny, _right_type, _right_parameter,outward_vec[3], _bias_y);

  // Use for the parametrization on edge pairs, currently generated on the fly
  // on the [0,1] interval
  Real rx_coord, sy_coord;

  std::vector<Node *> nodes(total_nodes); // can be done using .reserve as well
  unsigned node_id = 0;

  unsigned el_id = 0;
  Point newPt;
  Real r1_basis, r2_basis, s1_basis, s2_basis;
  for (unsigned idx = 0; idx < _nx; idx++)
  { // need more asserts here and possibly use the FP standards in MOOSE
    rx_coord = double(idx) / double(_nx - 1);
    r1_basis = 1 - rx_coord;
    r2_basis = rx_coord;

    for (unsigned idy = 0; idy < _ny; idy++)
    {
      sy_coord = double(idy) / double(_ny - 1);
      s1_basis = 1 - sy_coord;
      s2_basis = sy_coord;

      // this is the core of the algorithm and generates every internal point
      newPt = r1_basis * edge_right[idy] + r2_basis * edge_left[idy] + s1_basis * edge_bottom[idx] +
              s2_basis * edge_top[idx] - r1_basis * s1_basis * V00 - r1_basis * s2_basis * V01 -
              r2_basis * s1_basis * V10 - r2_basis * s2_basis * V11;

      nodes[node_id] = mesh->add_point(newPt, node_id);
      node_id++;
    }
  }

  for (unsigned idx = 0; idx < _nx - 1; idx++)
  {
    for (unsigned idy = 0; idy < _ny - 1; idy++)
    {
      Elem * elem = mesh->add_elem(new Quad4);
      elem->set_node(0) = nodes[idy + idx * _ny];
      elem->set_node(1) = nodes[idy + (idx + 1) * _ny];
      elem->set_node(2) = nodes[idy + 1 + (idx + 1) * _ny];
      elem->set_node(3) = nodes[idy + 1 + idx * _ny];
      el_id++;

      if (idy == 0) // add bottom boundary (boundary_id = 0)
        boundary_info.add_side(elem, 0, 0);

      if (idx == _nx - 2) // add right boundary (boundary_id = 3)
        boundary_info.add_side(elem, 1, 2);

      if (idy == _ny - 2) // add top boundary (boundary_id = 1)
        boundary_info.add_side(elem, 2, 1);

      if (idx == 0) // add left boundary (boundary_id = 2)
        boundary_info.add_side(elem, 3, 3);
    }
  }

  boundary_info.sideset_name(0) = "bottom";
  boundary_info.nodeset_name(0) = "bottom";

  boundary_info.sideset_name(3) = "right";
  boundary_info.nodeset_name(3) = "right";

  boundary_info.sideset_name(1) = "top";
  boundary_info.nodeset_name(1) = "top";

  boundary_info.sideset_name(2) = "left";
  boundary_info.nodeset_name(2) = "left";

  mesh->prepare_for_use();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::vector<Point>
TransfiniteMeshGenerator::getParsedEdge(const Point & P1, const Point & P2,
            const unsigned int & np, const std::string & parameter, const Real & bias)
{
  std::vector<Point> edge;
  std::vector<Real> param_vec;
  param_vec=getParametrization(1.0, np, 1.0);
  Real x_coord, y_coord, r_param;

  std::vector<std::string> param_coords;
  //std::vector<Real> yvec;
  MooseUtils::tokenize(parameter, param_coords, 1, "&&");


  for (unsigned int iter=0; iter<np; iter++)
  { r_param=param_vec[iter];
  _parsed_func->Parse(param_coords[0], "r");
  x_coord=_parsed_func->Eval(&r_param);
  _parsed_func->Parse(param_coords[1], "r");
  y_coord=_parsed_func->Eval(&r_param);
  edge.push_back(Point(x_coord, y_coord, 0.0));

  }
  return edge;
}


std::vector<Point>
TransfiniteMeshGenerator::getEdge(const Point & P1,
                                  const Point & P2,
                                  const unsigned int & np,
                                  const MooseEnum & type,
                                  const std::string & parameter,
                                  const Point & outward,
                                  const Real & bias)
{
  std::vector<Point> edge;
  std::vector<Real> param_vec;
  Real edge_length=(P2-P1).norm();
  param_vec=getParametrization(edge_length, np, bias);
  Real rx;

  switch (type)
  {
    case 1:
      for (unsigned iter = 0; iter < np; iter++)
      {
        rx=param_vec[iter];
        Point newPt = P1 * (1.0 - rx) + P2 * rx;
        edge.push_back(newPt);
      };
      break;
    case 2:
      for (unsigned iter = 0; iter < np; iter++)
      {
        Real height = MooseUtils::convert<Real>(parameter, true);
        rx = double(iter) / double(np - 1);
        Point P3 = computeMidPoint(P1, P2, height, outward);
        Real rad = computeRadius(P1, P2, P3);
        Point P0 = computeOrigin(P1, P2, P3);
        //need to shift to center of coordinates to find the corresponding radians
        Point x0 = (P1 - P0);
        Point x1 = (P2 - P0);
        //The below function should be updated to take as an argument a Point
        //and compute it in polar coordinates
        Real a = getPolarAngle(x0);
        Real b = getPolarAngle(x1);
        //The case when the edge spans quadrants 1 and 4 requires special treament
        //to periodically switch we compute the angle that needs added to one edge
        //to identify the entire edge span
        Real arclength=std::acos((x0*x1)/x0.norm()/x1.norm());
        if (abs(b-a)>M_PI) b=a+arclength;
        Real interval = getMapFromReference(rx, a, b);
        Real x = P0(0) + rad * std::cos(interval);
        Real y = P0(1) + rad * std::sin(interval);
        edge.push_back(Point(x, y, 0.0));
      };
      break;
    case 3:
    {
      std::vector<std::string> string_points;
      MooseUtils::tokenize(parameter, string_points, 1, "\n");
      for (unsigned iter = 0; iter < string_points.size(); iter++)
      {
        std::vector<Real> point_vals;
        MooseUtils::tokenizeAndConvert(string_points[iter], point_vals, " ");
        edge.push_back(Point(point_vals[0], point_vals[1], point_vals[2]));
      }
    }
    break;
    case 4:
      edge=getParsedEdge(P1,P2,np,parameter, bias);
      break;
  }
  return edge;
}

Real
TransfiniteMeshGenerator::getEdgeLength(const Point & P1, const Point & P2) const
{
  Point temp = P1 - P2;
  Real edge_len = temp.norm();

  return edge_len;
}

Real
TransfiniteMeshGenerator::getPolarAngle(const Point & Px) const
{
  Real x = Px(0);
  Real y = Px(1);
  // define quadrants
  const bool q1 = (x > 0 && y > 0);
  const bool q2 = (x < 0 && y > 0);
  const bool q3 = (x < 0 && y < 0);
  const bool q4 = (x > 0 && y < 0);

  Real angle = std::atan(std::abs(y) / std::abs(x));
  // compute angles via the inverse tangent
  // however the quadrants do not provide sufficient info
  // and we need to involve normals info as well
  if (q2)
    angle = M_PI-angle;
  if (q3)
    angle = M_PI+angle;
  if (q4)
    angle = 2*M_PI-angle;

  return angle;
}

Real
TransfiniteMeshGenerator::getMapToReference(const Real & x, const Real & a, const Real & b) const
{
  Real r = (x - a) / (b - a);
  return r;
}

Real
TransfiniteMeshGenerator::getMapFromReference(const Real & x, const Real & a, const Real & b) const
{
  Real r = x * (b - a) + a;
  return r;
}

std::vector<Real>
TransfiniteMeshGenerator::getParametrization(const Real & edge_length,
                          const unsigned int & np,
                          const Real & bias) const
{
  std::vector<Real> param_vec;
  Real rx=0.0;

  if (bias!=1.0)
    {
      Real step=0.0;
      Real factor=edge_length * (1.0 - std::abs(bias)) /(1.0 - std::pow(std::abs(bias), np-1));
      param_vec.push_back(rx);
      for (unsigned iter = 1; iter < np; iter++)
       {
        step = step + factor * std::pow(bias, double(iter-1));
        rx = getMapToReference(step, 0, edge_length);
        param_vec.push_back(rx);
       }
    }
  else
    {
     for (unsigned iter = 0; iter < np; iter++)
       {
       rx = double(iter) / double(np - 1);
        param_vec.push_back(rx);
       }
     }
  return param_vec;
}


Real
TransfiniteMeshGenerator::computeRadius(const Point & P1, const Point & P2, const Point & P3) const
{
  Point temp1 = P1 - P2;
  Real a2 = temp1.norm_sq(); // a is the distance from P1 to P2, but we only it squared
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
  Real a1, a2, A;
  if (abs(P2(0) - P1(0)) > 1e-15 || abs(P2(1) - P1(1)) > 1e-15)
  {
    a1 = (2 * P3(0) - 2 * P1(0));
    a2 = (2 * P3(1) - 2 * P1(1));
    A = P3(1) * P3(1) - P1(0) * P1(0) + P3(0) * P3(0) - P1(1) * P1(1);
  }
  else
  {
    a1 = (2 * P2(0) - 2 * P1(0));
    a2 = (2 * P2(1) - 2 * P1(1));
    A = P2(1) * P2(1) - P1(0) * P1(0) + P2(0) * P2(0) - P1(1) * P1(1);
  }
  // note we need to have two cases since it may happen that either P1,P2 or
  //  P1/P2,P3 are aligned with the system of coordinates
  const Real b1 = (2 * P3(0) - 2 * P2(0));
  const Real b2 = (2 * P3(1) - 2 * P2(1));
  const Real B = P3(1) * P3(1) - P2(0) * P2(0) + P3(0) * P3(0) - P2(1) * P2(1);

  const Real y0 = (a1 * b2) * (B / b2 - A * b1 / (a1 * b2)) / (a1 * b2 - a2 * b1);
  const Real x0 = (A - y0 * a2) / a1;

  // fill in and return the origin point
  Point P0 = Point(x0, y0, 0.0);
  return P0;
}

Point
TransfiniteMeshGenerator::computeMidPoint(const Point & P1,
                                          const Point & P2,
                                          const Real & height,
                                          const Point & outward) const
{
  const Real xm = (P1(0) + P2(0)) / 2;
  const Real ym = (P1(1) + P2(1)) / 2;
  // The arc can be inverted into the domain (concave) or outward (convex)
  // we use the convention that if the height is given as negative we seek a concave arc
  // if "height" is positive then we seek a convex arc
  const Real dist = abs(height);
  const int orient=(height >= 0) ? 1 : -1;
  Point MidPoint;

  // this accounts for lines aligned with the system of coordinates
  if ((abs(P2(0) - P1(0)) < 1e-15) || (abs(P2(1) - P1(1)) < 1e-15))
    MidPoint = Point(xm, ym , 0.0) + dist * orient* outward;

  // some of the cases are  not covered by this strategy and we need to use normals info
  // this is implemented and tested in the prototype and is soon to be updated
  if (abs(P2(0) - P1(0)) > 1e-15 && abs(P2(1) - P1(1)) > 1e-15)
  {
    // m is the slope of the line orthogonal to the midpoint
    const Real m = -(P2(0) - P1(0)) / (P2(1) - P1(1));
    // The equation to determine allows two solutions
    const Real x_temp = dist* sqrt((m * m + 1)) / (m * m + 1);
    const Real factor=orient*outward(0)+m*orient*outward(1);
    int direction=(factor >= 0) ? 1 : -1;

    MidPoint = Point(direction*x_temp + xm, direction*x_temp*m + ym, 0.0);

  }
  return MidPoint;
}
