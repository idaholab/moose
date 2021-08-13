//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMFuncs.h"

#include "MooseError.h"
#include "Conversion.h"

using namespace libMesh;

namespace Xfem
{

void
dunavant_rule2(const Real * wts,
               const Real * a,
               const Real * b,
               const unsigned int * permutation_ids,
               unsigned int n_wts,
               std::vector<Point> & points,
               std::vector<Real> & weights)
{
  // see libmesh/src/quadrature/quadrature_gauss.C
  // Figure out how many total points by summing up the entries
  // in the permutation_ids array, and resize the _points and _weights
  // vectors appropriately.
  unsigned int total_pts = 0;
  for (unsigned int p = 0; p < n_wts; ++p)
    total_pts += permutation_ids[p];

  // Resize point and weight vectors appropriately.
  points.resize(total_pts);
  weights.resize(total_pts);

  // Always insert into the points & weights vector relative to the offset
  unsigned int offset = 0;
  for (unsigned int p = 0; p < n_wts; ++p)
  {
    switch (permutation_ids[p])
    {
      case 1:
      {
        // The point has only a single permutation (the centroid!)
        // So we don't even need to look in the a or b arrays.
        points[offset + 0] = Point(1.0L / 3.0L, 1.0L / 3.0L);
        weights[offset + 0] = wts[p];

        offset += 1;
        break;
      }

      case 3:
      {
        // For this type of rule, don't need to look in the b array.
        points[offset + 0] = Point(a[p], a[p]);             // (a,a)
        points[offset + 1] = Point(a[p], 1.L - 2.L * a[p]); // (a,1-2a)
        points[offset + 2] = Point(1.L - 2.L * a[p], a[p]); // (1-2a,a)

        for (unsigned int j = 0; j < 3; ++j)
          weights[offset + j] = wts[p];

        offset += 3;
        break;
      }

      case 6:
      {
        // This type of point uses all 3 arrays...
        points[offset + 0] = Point(a[p], b[p]);
        points[offset + 1] = Point(b[p], a[p]);
        points[offset + 2] = Point(a[p], 1.L - a[p] - b[p]);
        points[offset + 3] = Point(1.L - a[p] - b[p], a[p]);
        points[offset + 4] = Point(b[p], 1.L - a[p] - b[p]);
        points[offset + 5] = Point(1.L - a[p] - b[p], b[p]);

        for (unsigned int j = 0; j < 6; ++j)
          weights[offset + j] = wts[p];

        offset += 6;
        break;
      }

      default:
        mooseError("Unknown permutation id: ", permutation_ids[p], "!");
    }
  }
}

void
stdQuadr2D(unsigned int nen, unsigned int iord, std::vector<std::vector<Real>> & sg2)
{
  // Purpose: get Guass integration points for 2D quad and tri elems
  // N.B. only works for n_qp <= 6

  Real lr4[4] = {-1.0, 1.0, -1.0, 1.0}; // libmesh order
  Real lz4[4] = {-1.0, -1.0, 1.0, 1.0};
  Real lr9[9] = {-1.0, 0.0, 1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0}; // libmesh order
  Real lz9[9] = {-1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
  Real lw9[9] = {25.0, 40.0, 25.0, 40.0, 64.0, 40.0, 25.0, 40.0, 25.0};

  if (nen == 4) // 2d quad element
  {
    if (iord == 1) // 1-point Gauss
    {
      sg2.resize(1);
      sg2[0].resize(3);
      sg2[0][0] = 0.0;
      sg2[0][1] = 0.0;
      sg2[0][2] = 4.0;
    }
    else if (iord == 2) // 2x2-point Gauss
    {
      sg2.resize(4);
      for (unsigned int i = 0; i < 4; ++i)
        sg2[i].resize(3);
      for (unsigned int i = 0; i < 4; ++i)
      {
        sg2[i][0] = (1 / sqrt(3)) * lr4[i];
        sg2[i][1] = (1 / sqrt(3)) * lz4[i];
        sg2[i][2] = 1.0;
      }
    }
    else if (iord == 3) // 3x3-point Gauss
    {
      sg2.resize(9);
      for (unsigned int i = 0; i < 9; ++i)
        sg2[i].resize(3);
      for (unsigned int i = 0; i < 9; ++i)
      {
        sg2[i][0] = sqrt(0.6) * lr9[i];
        sg2[i][1] = sqrt(0.6) * lz9[i];
        sg2[i][2] = (1.0 / 81.0) * lw9[i];
      }
    }
    else
      mooseError("Invalid quadrature order = " + Moose::stringify(iord) + " for quad elements");
  }
  else if (nen == 3) // triangle
  {
    if (iord == 1) // one-point Gauss
    {
      sg2.resize(1);
      sg2[0].resize(4);
      sg2[0][0] = 1.0 / 3.0;
      sg2[0][1] = 1.0 / 3.0;
      sg2[0][2] = 1.0 / 3.0;
      sg2[0][3] = 0.5;
    }
    else if (iord == 2) // three-point Gauss
    {
      sg2.resize(3);
      for (unsigned int i = 0; i < 3; ++i)
        sg2[i].resize(4);
      sg2[0][0] = 2.0 / 3.0;
      sg2[0][1] = 1.0 / 6.0;
      sg2[0][2] = 1.0 / 6.0;
      sg2[0][3] = 1.0 / 6.0;
      sg2[1][0] = 1.0 / 6.0;
      sg2[1][1] = 2.0 / 3.0;
      sg2[1][2] = 1.0 / 6.0;
      sg2[1][3] = 1.0 / 6.0;
      sg2[2][0] = 1.0 / 6.0;
      sg2[2][1] = 1.0 / 6.0;
      sg2[2][2] = 2.0 / 3.0;
      sg2[2][3] = 1.0 / 6.0;
    }
    else if (iord == 3) // four-point Gauss
    {
      sg2.resize(4);
      for (unsigned int i = 0; i < 4; ++i)
        sg2[i].resize(4);
      sg2[0][0] = 1.5505102572168219018027159252941e-01;
      sg2[0][1] = 1.7855872826361642311703513337422e-01;
      sg2[0][2] = 1.0 - sg2[0][0] - sg2[0][1];
      sg2[0][3] = 1.5902069087198858469718450103758e-01;

      sg2[1][0] = 6.4494897427831780981972840747059e-01;
      sg2[1][1] = 7.5031110222608118177475598324603e-02;
      sg2[1][2] = 1.0 - sg2[1][0] - sg2[1][1];
      sg2[1][3] = 9.0979309128011415302815498962418e-02;

      sg2[2][0] = 1.5505102572168219018027159252941e-01;
      sg2[2][1] = 6.6639024601470138670269327409637e-01;
      sg2[2][2] = 1.0 - sg2[2][0] - sg2[2][1];
      sg2[2][3] = 1.5902069087198858469718450103758e-01;

      sg2[3][0] = 6.4494897427831780981972840747059e-01;
      sg2[3][1] = 2.8001991549907407200279599420481e-01;
      sg2[3][2] = 1.0 - sg2[3][0] - sg2[3][1];
      sg2[3][3] = 9.0979309128011415302815498962418e-02;
    }
    else if (iord == 4) // six-point Guass
    {
      const unsigned int n_wts = 2;
      const Real wts[n_wts] = {1.1169079483900573284750350421656140e-01L,
                               5.4975871827660933819163162450105264e-02L};

      const Real a[n_wts] = {4.4594849091596488631832925388305199e-01L,
                             9.1576213509770743459571463402201508e-02L};

      const Real b[n_wts] = {0., 0.}; // not used
      const unsigned int permutation_ids[n_wts] = {3, 3};

      std::vector<Point> points;
      std::vector<Real> weights;
      dunavant_rule2(wts, a, b, permutation_ids, n_wts, points, weights); // 6 total points

      sg2.resize(6);
      for (unsigned int i = 0; i < 6; ++i)
        sg2[i].resize(4);
      for (unsigned int i = 0; i < 6; ++i)
      {
        sg2[i][0] = points[i](0);
        sg2[i][1] = points[i](1);
        sg2[i][2] = 1.0 - points[i](0) - points[i](1);
        sg2[i][3] = weights[i];
      }
    }
    else
      mooseError("Invalid quadrature order = " + Moose::stringify(iord) + " for triangle elements");
  }
  else
    mooseError("Invalid 2D element type");
}

void
wissmannPoints(unsigned int nqp, std::vector<std::vector<Real>> & wss)
{
  if (nqp == 6)
  {
    wss.resize(6);
    for (unsigned int i = 0; i < 6; ++i)
      wss[i].resize(3);
    wss[0][0] = 0.0;
    wss[0][1] = 0.0;
    wss[0][2] = 1.1428571428571428;

    wss[1][0] = 0.0;
    wss[1][1] = 9.6609178307929590e-01;
    wss[1][2] = 4.3956043956043956e-01;

    wss[2][0] = 8.5191465330460049e-01;
    wss[2][1] = 4.5560372783619284e-01;
    wss[2][2] = 5.6607220700753210e-01;

    wss[3][0] = -wss[2][0];
    wss[3][1] = wss[2][1];
    wss[3][2] = wss[2][2];

    wss[4][0] = 6.3091278897675402e-01;
    wss[4][1] = -7.3162995157313452e-01;
    wss[4][2] = 6.4271900178367668e-01;

    wss[5][0] = -wss[4][0];
    wss[5][1] = wss[4][1];
    wss[5][2] = wss[4][2];
  }
  else
    mooseError("Unknown Wissmann quadrature type");
}

void
shapeFunc2D(unsigned int nen,
            std::vector<Real> & ss,
            std::vector<Point> & xl,
            std::vector<std::vector<Real>> & shp,
            Real & xsj,
            bool natl_flg)
{
  // Get shape functions and derivatives
  Real s[4] = {-0.5, 0.5, 0.5, -0.5};
  Real t[4] = {-0.5, -0.5, 0.5, 0.5};

  if (nen == 4) // quad element
  {
    Real xs[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    Real sx[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    for (unsigned int i = 0; i < 4; ++i)
    {
      shp[i][2] = (0.5 + s[i] * ss[0]) * (0.5 + t[i] * ss[1]);
      shp[i][0] = s[i] * (0.5 + t[i] * ss[1]);
      shp[i][1] = t[i] * (0.5 + s[i] * ss[0]);
    }
    for (unsigned int i = 0; i < 2; ++i) // x, y
    {
      for (unsigned int j = 0; j < 2; ++j) // xi, eta
      {
        xs[i][j] = 0.0;
        for (unsigned int k = 0; k < nen; ++k)
          xs[i][j] += xl[k](i) * shp[k][j];
      }
    }
    xsj = xs[0][0] * xs[1][1] - xs[0][1] * xs[1][0]; // det(j)
    if (natl_flg == false)                           // get global derivatives
    {
      Real temp = 1.0 / xsj;
      sx[0][0] = xs[1][1] * temp; // inv(j)
      sx[1][1] = xs[0][0] * temp;
      sx[0][1] = -xs[0][1] * temp;
      sx[1][0] = -xs[1][0] * temp;
      for (unsigned int i = 0; i < nen; ++i)
      {
        temp = shp[i][0] * sx[0][0] + shp[i][1] * sx[1][0];
        shp[i][1] = shp[i][0] * sx[0][1] + shp[i][1] * sx[1][1];
        shp[i][0] = temp;
      }
    }
  }
  else if (nen == 3) // triangle element
  {
    // x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2)
    Point x13 = xl[2] - xl[0];
    Point x23 = xl[2] - xl[1];
    Point cross_prod = x13.cross(x23);
    xsj = cross_prod.norm();
    Real xsjr = 1.0;
    if (xsj != 0.0)
      xsjr = 1.0 / xsj;
    // xsj  *= 0.5; // we do not have this 0.5 here because in stdQuad2D the sum of all weights in
    // tri is 0.5
    shp[0][2] = ss[0];
    shp[1][2] = ss[1];
    shp[2][2] = ss[2];
    if (natl_flg == false) // need global drivatives
    {
      shp[0][0] = (xl[1](1) - xl[2](1)) * xsjr;
      shp[0][1] = (xl[2](0) - xl[1](0)) * xsjr;
      shp[1][0] = (xl[2](1) - xl[0](1)) * xsjr;
      shp[1][1] = (xl[0](0) - xl[2](0)) * xsjr;
      shp[2][0] = (xl[0](1) - xl[1](1)) * xsjr;
      shp[2][1] = (xl[1](0) - xl[0](0)) * xsjr;
    }
    else
    {
      shp[0][0] = 1.0;
      shp[0][1] = 0.0;
      shp[1][0] = 0.0;
      shp[1][1] = 1.0;
      shp[2][0] = -1.0;
      shp[2][1] = -1.0;
    }
  }
  else
    mooseError("ShapeFunc2D only works for linear quads and tris!");
}

double
r8vec_norm(int n, double a[])
{
  //    John Burkardt geometry.cpp
  double v = 0.0;
  for (int i = 0; i < n; ++i)
    v = v + a[i] * a[i];
  v = std::sqrt(v);
  return v;
}

void
r8vec_copy(int n, double a1[], double a2[])
{
  //    John Burkardt geometry.cpp
  for (int i = 0; i < n; ++i)
    a2[i] = a1[i];
  return;
}

bool
r8vec_eq(int n, double a1[], double a2[])
{
  //    John Burkardt geometry.cpp
  for (int i = 0; i < n; ++i)
    if (a1[i] != a2[i])
      return false;
  return true;
}

double
r8vec_dot_product(int n, double a1[], double a2[])
{
  //    John Burkardt geometry.cpp
  double value = 0.0;
  for (int i = 0; i < n; ++i)
    value += a1[i] * a2[i];
  return value;
}

bool
line_exp_is_degenerate_nd(int dim_num, double p1[], double p2[])
{
  //    John Burkardt geometry.cpp
  bool value;
  value = r8vec_eq(dim_num, p1, p2);
  return value;
}

int
plane_normal_line_exp_int_3d(
    double pp[3], double normal[3], double p1[3], double p2[3], double pint[3])
{
//    John Burkardt geometry.cpp
//  Parameters:
//
//    Input, double PP[3], a point on the plane.
//
//    Input, double NORMAL[3], a normal vector to the plane.
//
//    Input, double P1[3], P2[3], two distinct points on the line.
//
//    Output, double PINT[3], the coordinates of a
//    common point of the plane and line, when IVAL is 1 or 2.
//
//    Output, integer PLANE_NORMAL_LINE_EXP_INT_3D, the kind of intersection;
//    0, the line and plane seem to be parallel and separate;
//    1, the line and plane intersect at a single point;
//    2, the line and plane seem to be parallel and joined.
#define DIM_NUM 3

  double direction[DIM_NUM];
  int ival;
  double temp;
  double temp2;
  //
  //  Make sure the line is not degenerate.
  if (line_exp_is_degenerate_nd(DIM_NUM, p1, p2))
    mooseError("PLANE_NORMAL_LINE_EXP_INT_3D - Fatal error!  The line is degenerate.");
  //
  //  Make sure the plane normal vector is a unit vector.
  temp = r8vec_norm(DIM_NUM, normal);
  if (temp == 0.0)
    mooseError("PLANE_NORMAL_LINE_EXP_INT_3D - Fatal error!  The normal vector of the plane is "
               "degenerate.");

  for (unsigned int i = 0; i < DIM_NUM; ++i)
    normal[i] = normal[i] / temp;
  //
  //  Determine the unit direction vector of the line.
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    direction[i] = p2[i] - p1[i];
  temp = r8vec_norm(DIM_NUM, direction);

  for (unsigned int i = 0; i < DIM_NUM; ++i)
    direction[i] = direction[i] / temp;
  //
  //  If the normal and direction vectors are orthogonal, then
  //  we have a special case to deal with.
  if (r8vec_dot_product(DIM_NUM, normal, direction) == 0.0)
  {
    temp = 0.0;
    for (unsigned int i = 0; i < DIM_NUM; ++i)
      temp = temp + normal[i] * (p1[i] - pp[i]);

    if (temp == 0.0)
    {
      ival = 2;
      r8vec_copy(DIM_NUM, p1, pint);
    }
    else
    {
      ival = 0;
      for (unsigned int i = 0; i < DIM_NUM; ++i)
        pint[i] = 1.0e20; // dummy huge value
    }
    return ival;
  }
  //
  //  Determine the distance along the direction vector to the intersection point.
  temp = 0.0;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    temp = temp + normal[i] * (pp[i] - p1[i]);
  temp2 = 0.0;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    temp2 = temp2 + normal[i] * direction[i];

  ival = 1;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    pint[i] = p1[i] + temp * direction[i] / temp2;

  return ival;
#undef DIM_NUM
}

double
polyhedron_volume_3d(
    double coord[], int order_max, int face_num, int node[], int /*node_num*/, int order[])
//
//  Purpose:
//
//    POLYHEDRON_VOLUME_3D computes the volume of a polyhedron in 3D.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    21 August 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double COORD[NODE_NUM*3], the 3D coordinates of the vertices.
//    The vertices may be listed in any order.
//
//    Input, int ORDER_MAX, the maximum number of vertices that make
//    up a face of the polyhedron.
//
//    Input, int FACE_NUM, the number of faces of the polyhedron.
//
//    Input, int NODE[FACE_NUM*ORDER_MAX].  Face I is defined by
//    the vertices NODE(I,1) through NODE(I,ORDER(I)).  These vertices
//    are listed in neighboring order.
//
//    Input, int NODE_NUM, the number of points stored in COORD.
//
//    Input, int ORDER[FACE_NUM], the number of vertices making up
//    each face.
//
//    Output, double POLYHEDRON_VOLUME_3D, the volume of the polyhedron.
//
{
#define DIM_NUM 3

  int face;
  int n1;
  int n2;
  int n3;
  double term;
  int v;
  double volume;
  double x1;
  double x2;
  double x3;
  double y1;
  double y2;
  double y3;
  double z1;
  double z2;
  double z3;
  //
  volume = 0.0;
  //
  //  Triangulate each face.
  //
  for (face = 0; face < face_num; face++)
  {
    n3 = node[order[face] - 1 + face * order_max];
    x3 = coord[0 + n3 * 3];
    y3 = coord[1 + n3 * 3];
    z3 = coord[2 + n3 * 3];

    for (v = 0; v < order[face] - 2; v++)
    {
      n1 = node[v + face * order_max];
      x1 = coord[0 + n1 * 3];
      y1 = coord[1 + n1 * 3];
      z1 = coord[2 + n1 * 3];

      n2 = node[v + 1 + face * order_max];
      x2 = coord[0 + n2 * 3];
      y2 = coord[1 + n2 * 3];
      z2 = coord[2 + n2 * 3];

      term =
          x1 * y2 * z3 - x1 * y3 * z2 + x2 * y3 * z1 - x2 * y1 * z3 + x3 * y1 * z2 - x3 * y2 * z1;

      volume = volume + term;
    }
  }

  volume = volume / 6.0;

  return volume;
#undef DIM_NUM
}

void
i4vec_zero(int n, int a[])
//
//  Purpose:
//
//    I4VEC_ZERO zeroes an I4VEC.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 August 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Output, int A[N], a vector of zeroes.
//
{
  int i;

  for (i = 0; i < n; i++)
  {
    a[i] = 0;
  }
  return;
}

void
normalizePoint(Point & p)
{
  Real len = p.norm();
  if (len > tol)
    p = (1.0 / len) * p;
  else
    p.zero();
}

void
normalizePoint(EFAPoint & p)
{
  Real len = p.norm();
  if (len > tol)
    p *= (1.0 / len);
}

double
r8_acos(double c)
//
//  Purpose:
//
//    R8_ACOS computes the arc cosine function, with argument truncation.
//
//  Discussion:
//
//    If you call your system ACOS routine with an input argument that is
//    outside the range [-1.0, 1.0 ], you may get an unpleasant surprise.
//    This routine truncates arguments outside the range.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    13 June 2002
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double C, the argument, the cosine of an angle.
//
//    Output, double R8_ACOS, an angle whose cosine is C.
//
{
#define PI 3.141592653589793

  double value;

  if (c <= -1.0)
  {
    value = PI;
  }
  else if (1.0 <= c)
  {
    value = 0.0;
  }
  else
  {
    value = acos(c);
  }
  return value;
#undef PI
}

double
angle_rad_3d(double p1[3], double p2[3], double p3[3])
//
//  Purpose:
//
//    ANGLE_RAD_3D returns the angle between two vectors in 3D.
//
//  Discussion:
//
//    The routine always computes the SMALLER of the two angles between
//    two vectors.  Thus, if the vectors make an (exterior) angle of 200
//    degrees, the (interior) angle of 160 is reported.
//
//    X dot Y = Norm(X) * Norm(Y) * Cos ( Angle(X,Y) )
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    20 June 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double P1[3], P2[3], P3[3], points defining an angle.
//    The rays are P1 - P2 and P3 - P2.
//
//    Output, double ANGLE_RAD_3D, the angle between the two vectors, in radians.
//    This value will always be between 0 and PI.  If either vector has
//    zero length, then the angle is returned as zero.
//
{
#define DIM_NUM 3

  double dot;
  int i;
  double v1norm;
  double v2norm;
  double value;

  v1norm = 0.0;
  for (i = 0; i < DIM_NUM; i++)
  {
    v1norm = v1norm + pow(p1[i] - p2[i], 2);
  }
  v1norm = sqrt(v1norm);

  if (v1norm == 0.0)
  {
    value = 0.0;
    return value;
  }

  v2norm = 0.0;
  for (i = 0; i < DIM_NUM; i++)
  {
    v2norm = v2norm + pow(p3[i] - p2[i], 2);
  }
  v2norm = sqrt(v2norm);

  if (v2norm == 0.0)
  {
    value = 0.0;
    return value;
  }

  dot = 0.0;
  for (i = 0; i < DIM_NUM; i++)
  {
    dot = dot + (p1[i] - p2[i]) * (p3[i] - p2[i]);
  }

  value = r8_acos(dot / (v1norm * v2norm));

  return value;
#undef DIM_NUM
}

bool
intersectSegmentWithCutLine(const Point & segment_point1,
                            const Point & segment_point2,
                            const std::pair<Point, Point> & cutting_line_points,
                            const Real & cutting_line_fraction,
                            Real & segment_intersection_fraction)
{
  // Use the algorithm described here to determine whether a line segment is intersected
  // by a cutting line, and to compute the fraction along that line where the intersection
  // occurs:
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  bool cut_segment = false;
  Point seg_dir = segment_point2 - segment_point1;
  Point cut_dir = cutting_line_points.second - cutting_line_points.first;
  Point cut_start_to_seg_start = segment_point1 - cutting_line_points.first;

  Real cut_dir_cross_seg_dir = crossProduct2D(cut_dir, seg_dir);

  if (std::abs(cut_dir_cross_seg_dir) > Xfem::tol)
  {
    // Fraction of the distance along the cutting segment where it intersects the edge segment
    Real cut_int_frac = crossProduct2D(cut_start_to_seg_start, seg_dir) / cut_dir_cross_seg_dir;

    if (cut_int_frac >= 0.0 && cut_int_frac <= cutting_line_fraction)
    { // Cutting segment intersects the line of the edge segment, but the intersection point may be
      // outside the segment
      Real int_frac = crossProduct2D(cut_start_to_seg_start, cut_dir) / cut_dir_cross_seg_dir;
      if (int_frac >= 0.0 && int_frac <= 1.0)
      {
        cut_segment = true;
        segment_intersection_fraction = int_frac;
      }
    }
  }
  return cut_segment;
}

Real
crossProduct2D(const Point & point_a, const Point & point_b)
{
  return (point_a(0) * point_b(1) - point_b(0) * point_a(1));
}

Real
pointSegmentDistance(const Point & x0, const Point & x1, const Point & x2, Point & xp)
{
  Point dx = x2 - x1;
  Real m2 = dx * dx;
  if (m2 == 0)
    mooseError("In XFEMFuncs::pointSegmentDistance(), x0 and x1 should be two different points.");
  // find parameter coordinate of closest point on segment
  Real s12 = (x2 - x0) * dx / m2;
  if (s12 < 0)
    s12 = 0;
  else if (s12 > 1)
    s12 = 1;
  // and find the distance
  xp = s12 * x1 + (1 - s12) * x2;
  return std::sqrt((x0 - xp) * (x0 - xp));
}

Real
pointTriangleDistance(const Point & x0,
                      const Point & x1,
                      const Point & x2,
                      const Point & x3,
                      Point & xp,
                      unsigned int & region)
{
  Point x13 = x1 - x3, x23 = x2 - x3, x03 = x0 - x3;
  Real m13 = x13 * x13, m23 = x23 * x23, d = x13 * x23;
  Real invdet = 1.0 / std::max(m13 * m23 - d * d, 1e-30);
  Real a = x13 * x03, b = x23 * x03;

  Real w23 = invdet * (m23 * a - d * b);
  Real w31 = invdet * (m13 * b - d * a);
  Real w12 = 1 - w23 - w31;
  if (w23 >= 0 && w31 >= 0 && w12 >= 0)
  { // if we're inside the triangle
    region = 0;
    xp = w23 * x1 + w31 * x2 + w12 * x3;
    return std::sqrt((x0 - xp) * (x0 - xp));
  }
  else
  {
    if (w23 > 0) // this rules out edge 2-3 for us
    {
      Point xp1, xp2;
      Real distance_12 = pointSegmentDistance(x0, x1, x2, xp1);
      Real distance_13 = pointSegmentDistance(x0, x1, x3, xp2);
      Real distance_1 = std::sqrt((x0 - x1) * (x0 - x1));
      if (std::min(distance_12, distance_13) < distance_1)
      {
        if (distance_12 < distance_13)
        {
          region = 4;
          xp = xp1;
          return distance_12;
        }
        else
        {
          region = 6;
          xp = xp2;
          return distance_13;
        }
      }
      else
      {
        region = 1;
        xp = x1;
        return distance_1;
      }
    }
    else if (w31 > 0) // this rules out edge 1-3
    {
      Point xp1, xp2;
      Real distance_12 = pointSegmentDistance(x0, x1, x2, xp1);
      Real distance_23 = pointSegmentDistance(x0, x2, x3, xp2);
      Real distance_2 = std::sqrt((x0 - x2) * (x0 - x2));
      if (std::min(distance_12, distance_23) < distance_2)
      {
        if (distance_12 < distance_23)
        {
          region = 4;
          xp = xp1;
          return distance_12;
        }
        else
        {
          region = 5;
          xp = xp2;
          return distance_23;
        }
      }
      else
      {
        region = 2;
        xp = x2;
        return distance_2;
      }
    }
    else // w12 must be >0, ruling out edge 1-2
    {
      Point xp1, xp2;
      Real distance_23 = pointSegmentDistance(x0, x2, x3, xp1);
      Real distance_31 = pointSegmentDistance(x0, x3, x1, xp2);
      Real distance_3 = std::sqrt((x0 - x3) * (x0 - x3));
      if (std::min(distance_23, distance_31) < distance_3)
      {
        if (distance_23 < distance_31)
        {
          region = 5;
          xp = xp1;
          return distance_23;
        }
        else
        {
          region = 6;
          xp = xp2;
          return distance_31;
        }
      }
      else
      {
        region = 3;
        xp = x3;
        return distance_3;
      }
    }
  }
  mooseError("Cannot find closest location in XFEMFuncs::pointTriangleDistance().");
}

bool
intersectWithEdge(const Point & p1,
                  const Point & p2,
                  const std::vector<Point> & vertices,
                  Point & pint)
{
  bool has_intersection = false;

  if (vertices.size() != 3)
    mooseError("The number of vertices of cutting element must be 3.");

  Plane elem_plane(vertices[0], vertices[1], vertices[2]);
  Point point = vertices[0];
  Point normal = elem_plane.unit_normal(point);

  std::array<Real, 3> plane_point = {{point(0), point(1), point(2)}};
  std::array<Real, 3> planenormal = {{normal(0), normal(1), normal(2)}};
  std::array<Real, 3> edge_point1 = {{p1(0), p1(1), p1(2)}};
  std::array<Real, 3> edge_point2 = {{p2(0), p2(1), p2(2)}};
  std::array<Real, 3> cut_point = {{0.0, 0.0, 0.0}};

  if (Xfem::plane_normal_line_exp_int_3d(
          &plane_point[0], &planenormal[0], &edge_point1[0], &edge_point2[0], &cut_point[0]) == 1)
  {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if (isInsideCutPlane(vertices, temp_p) && isInsideEdge(p1, p2, temp_p))
    {
      pint = temp_p;
      has_intersection = true;
    }
  }

  return has_intersection;
}

bool
isInsideEdge(const Point & p1, const Point & p2, const Point & p)
{
  Real dotp1 = (p1 - p) * (p2 - p1);
  Real dotp2 = (p2 - p) * (p2 - p1);
  return (dotp1 * dotp2 <= 0.0);
}

Real
getRelativePosition(const Point & p1, const Point & p2, const Point & p)
{
  Real full_len = (p2 - p1).norm();
  Real len_p1_p = (p - p1).norm();
  return len_p1_p / full_len;
}

bool
isInsideCutPlane(const std::vector<Point> & vertices, const Point & p)
{
  unsigned int n_node = vertices.size();

  if (n_node != 3)
    mooseError("The number of vertices of cutting element must be 3.");

  Plane elem_plane(vertices[0], vertices[1], vertices[2]);
  Point normal = elem_plane.unit_normal(vertices[0]);

  bool inside = false;
  unsigned int counter = 0;

  for (unsigned int i = 0; i < n_node; ++i)
  {
    unsigned int iplus1 = (i < n_node - 1 ? i + 1 : 0);
    Point middle2p = p - 0.5 * (vertices[i] + vertices[iplus1]);
    const Point side_tang = vertices[iplus1] - vertices[i];
    Point side_norm = side_tang.cross(normal);

    normalizePoint(middle2p);
    normalizePoint(side_norm);

    if (middle2p * side_norm <= 0)
      counter += 1;
  }

  if (counter == n_node)
    inside = true;
  return inside;
}

} // namespace XFEM
