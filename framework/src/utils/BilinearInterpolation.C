//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BilinearInterpolation.h"
#include "libmesh/int_range.h"

int BilinearInterpolation::_file_number = 0;

BilinearInterpolation::BilinearInterpolation(const std::vector<Real> & x1,
                                             const std::vector<Real> & x2,
                                             const ColumnMajorMatrix & y)
  : BidimensionalInterpolation(x1, x2), _zSurface(y)
{}

void
BilinearInterpolation::getNeighborIndices(const std::vector<Real> & inArr,
                                          Real x,
                                          unsigned int & lowerX,
                                          unsigned int & upperX) const
{
  int N = inArr.size();
  if (x <= inArr[0])
  {
    lowerX = 0;
    upperX = 0;
  }
  else if (x >= inArr[N - 1])
  {
    lowerX = N - 1;
    upperX = N - 1;
  }
  else
  {
    for (const auto i : make_range(1, N))
    {
      if (x < inArr[i])
      {
        lowerX = i - 1;
        upperX = i;
        break;
      }
      else if (x == inArr[i])
      {
        lowerX = i;
        upperX = i;
        break;
      }
    }
  }
}

Real
BilinearInterpolation::sample(Real s1, Real s2) const
{
  return sampleInternal(s1, s2);
}

ADReal
BilinearInterpolation::sample(const ADReal & s1, const ADReal & s2) const
{
  return sampleInternal(s1, s2);
}

template <typename T>
T
BilinearInterpolation::sampleInternal(T & s1, T & s2) const
{
  // first find 4 neighboring points
  unsigned int lx = 0; // index of x coordinate of adjacent grid point to left of P
  unsigned int ux = 0; // index of x coordinate of adjacent grid point to right of P
  getNeighborIndices(_x1, MetaPhysicL::raw_value(s1), lx, ux);

  unsigned int ly = 0; // index of y coordinate of adjacent grid point below P
  unsigned int uy = 0; // index of y coordinate of adjacent grid point above P
  getNeighborIndices(_x2, MetaPhysicL::raw_value(s2), ly, uy);

  const Real & fQ11 = _zSurface(ly, lx);
  const Real & fQ21 = _zSurface(ly, ux);
  const Real & fQ12 = _zSurface(uy, lx);
  const Real & fQ22 = _zSurface(uy, ux);

  // if point exactly found on a node do not interpolate
  if ((lx == ux) && (ly == uy))
  {
    return fQ11;
  }

  const auto & x = s1;
  const auto & y = s2;
  const Real & x1 = _x1[lx];
  const Real & x2 = _x1[ux];
  const Real & y1 = _x2[ly];
  const Real & y2 = _x2[uy];

  // if x1 lies exactly on an xAxis node do linear interpolation
  if (lx == ux)
  {
    return fQ11 + (fQ12 - fQ11) * (y - y1) / (y2 - y1);
  }

  // if x2 lies exactly on an yAxis node do linear interpolation
  if (ly == uy)
  {
    return fQ11 + (fQ21 - fQ11) * (x - x1) / (x2 - x1);
  }

  auto fxy = fQ11 * (x2 - x) * (y2 - y);
  fxy += fQ21 * (x - x1) * (y2 - y);
  fxy += fQ12 * (x2 - x) * (y - y1);
  fxy += fQ22 * (x - x1) * (y - y1);
  fxy /= ((x2 - x1) * (y2 - y1));
  return fxy;
}

Real
BilinearInterpolation::sampleDerivative(Real s1, Real s2, unsigned int deriv_var) const
{
  // first find 4 neighboring points
  unsigned int lx = 0; // index of x coordinate of adjacent grid point to left of P
  unsigned int ux = 0; // index of x coordinate of adjacent grid point to right of P
  getNeighborIndices(_x1, MetaPhysicL::raw_value(s1), lx, ux);

  unsigned int ly = 0; // index of y coordinate of adjacent grid point below P
  unsigned int uy = 0; // index of y coordinate of adjacent grid point above P
  getNeighborIndices(_x2, MetaPhysicL::raw_value(s2), ly, uy);

  const Real & fQ11 = _zSurface(ly, lx);
  const Real & fQ21 = _zSurface(ly, ux);
  const Real & fQ12 = _zSurface(uy, lx);
  const Real & fQ22 = _zSurface(uy, ux);

  const auto & x = s1;
  const auto & y = s2;
  const Real & x1 = _x1[lx];
  const Real & x2 = _x1[ux];
  const Real & y1 = _x2[ly];
  const Real & y2 = _x2[uy];

  if ((lx == 0) && (ly == 0)) // if at bottom left node, take average slope of four boxes
  {
    const auto & fQ13 = _zSurface(ly + 1, lx); // fQ at (x1,y3)
    const auto & fQ31 = _zSurface(ly, lx + 1); //fQ at (x3,y1)
    const auto & fQ33 = _zSurface(ly + 1, lx + 1); //fQ at (x3,y3)
    const Real & x3 = _x1[lx + 1]; // ux value
    const Real & y3 = _x2[ly + 1]; // uy value

    if (deriv_var == 1)
    {
      auto dfdx = fQ11 * (y - y3);
      dfdx += fQ31 * (y3 - y);
      dfdx += fQ13 * (y1 - y);
      dfdx += fQ33 * (y - y1);
      dfdx /= ((x3 - x1) * (y3 - y1));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ11 * (x - x3);
      dfdy += fQ31 * (x1 - x);
      dfdy += fQ13 * (x3 - x);
      dfdy += fQ33 * (x - x1);
      dfdy /= ((x3 - x1) * (y3 - y1));
      if (std::isnan(dfdy))
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  else if ((ly == 0) && (ux == 99)) // if at bottom right node
  {
    const auto & fQ01 = _zSurface(ly, lx - 1); // fQ at (x0,y1)
    const auto & fQ03 = _zSurface(ly + 1, lx - 1); // fQ at (x0,y3)
    const auto & fQ23 = _zSurface(ly + 1, lx); // fQ at (x2,y3)
    const Real & x0 = _x1[lx - 1]; // lx value
    const Real & y3 = _x2[ly + 1]; // uy value

    if (deriv_var == 1)
    {
      auto dfdx = fQ01 * (y - y3);
      dfdx += fQ21 * (y3 - y);
      dfdx += fQ03 * (y1 - y);
      dfdx += fQ23 * (y - y1);
      dfdx /= ((x2 - x0) * (y3 - y1));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ01 * (x - x2);
      dfdy += fQ21 * (x0 - x);
      dfdy += fQ03 * (x2 - x);
      dfdy += fQ23 * (x - x0);
      dfdy /= ((x2 - x0) * (y3 - y1));
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  else if ((uy == 99) && (lx == 0)) // if at top left node
  {
    const auto & fQ10 = _zSurface(ly - 1, lx); // fQ at (x1,y0)
    const auto & fQ30 = _zSurface(ly + 1, lx + 1); // fQ at (x3,y0)
    const auto & fQ32 = _zSurface(ly, lx + 1); // fQ at (x3,y2)
    const Real & x3 = _x1[lx + 1]; // ux value
    const Real & y0 = _x2[ly - 1]; // ly value

    if (deriv_var == 1)
    {
      auto dfdx = fQ10 * (y - y2);
      dfdx += fQ30 * (y2 - y);
      dfdx += fQ12 * (y0 - y);
      dfdx += fQ32 * (y - y0);
      dfdx /= ((x3 - x1) * (y2 - y0));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ10 * (x - x3);
      dfdy += fQ30 * (x1 - x);
      dfdy += fQ12 * (x3 - x);
      dfdy += fQ32 * (x - x1);
      dfdy /= ((x3 - x1) * (y2 - y0));
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  else if ((uy == 99) && (ux == 99)) // if at top right node
  {
    const auto & fQ00 = _zSurface(ly - 1, lx - 1); // fQ at (x0,y0)
    const auto & fQ20 = _zSurface(ly - 1, lx); // fQ at (x2,y0)
    const auto & fQ02 = _zSurface(ly, lx - 1); // fQ at (x0,y2)
    const Real & x0 = _x1[lx - 1]; // lx value
    const Real & y0 = _x2[ly - 1]; // ly value

    if (deriv_var == 1)
    {
      auto dfdx = fQ00 * (y - y2);
      dfdx += fQ20 * (y2 - y);
      dfdx += fQ02 * (y0 - y);
      dfdx += fQ22 * (y - y0);
      dfdx /= ((x2 - x0) * (y2 - y0));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ00 * (x - x2);
      dfdy += fQ20 * (x0 - x);
      dfdy += fQ02 * (x2 - x);
      dfdy += fQ22 * (x - x0);
      dfdy /= ((x2 - x0) * (y2 - y0));
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }

  else if ((uy == 99) && (ly == uy) && (ux == lx)) // when along top bound and ux=lx, uy=ly
  {
    const auto & fQ00 = _zSurface(uy - 1, lx - 1); // fQ at (x0,y0)
    const auto & fQ01 = _zSurface(uy, lx - 1); // fQ at (x0,y1)
    const auto & fQ30 = _zSurface(uy - 1, lx + 1); // fQ at (x3,y0)
    const auto & fQ31 = _zSurface(uy, lx + 1); // fQ at (x3,y1)

    const Real & x0 = _x1[lx - 1]; // lx value
    const Real & x3 = _x1[lx + 1]; // ux value
    const Real & y0 = _x2[ly - 1]; // ly value

    if (deriv_var == 1)
    {
      auto dfdx = fQ00 * (y - y1);
      dfdx += fQ30 * (y1 - y);
      dfdx += fQ01 * (y0 - y);
      dfdx += fQ31 * (y - y0);
      dfdx /= ((x3 - x0) * (y1 - y0));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ00 * (x - x3);
      dfdy += fQ30 * (x0 - x);
      dfdy += fQ01 * (x3 - x);
      dfdy += fQ31 * (x - x0);
      dfdy /= ((x3 - x0) * (y1 - y0));
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  else if ((uy == 0) && (ly == uy) && (ux == lx)) // when along bottom bound and ux=lx, uy=ly
  {
    const auto & fQ01 = _zSurface(ly, lx - 1); // fQ at (x0,y1)
    const auto & fQ03 = _zSurface(ly + 1, lx - 1); // fQ at (x0,y3)
    const auto & fQ31 = _zSurface(ly, lx + 1); // fQ at (x3,y1)
    const auto & fQ33 = _zSurface(ly + 1, lx + 1); // fQ at (x3,y3)

    const Real & x0 = _x1[lx - 1]; // lx value
    const Real & x3 = _x1[lx + 1]; // ux value
    const Real & y3 = _x2[ly + 1]; // uy value

    if (deriv_var == 1)
    {
      auto dfdx = fQ01 * (y - y3);
      dfdx += fQ31 * (y3 - y);
      dfdx += fQ03 * (y1 - y);
      dfdx += fQ33 * (y - y1);
      dfdx /= ((x3 - x0) * (y3 - y1));
      return dfdx / 4;
    }
    else if (deriv_var == 2)
    {
      auto dfdy = fQ01 * (x - x3);
      dfdy += fQ31 * (x0 - x);
      dfdy += fQ03 * (x3 - x);
      dfdy += fQ33 * (x - x0);
      dfdy /= ((x3 - x0) * (y3 - y1));
      return dfdy / 4;
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  // calculate derivative wrt to x
  if (deriv_var == 1)
  {
    // Find derivative when on interval between two nodes
    if (y == y1)
    {
      return (fQ21 - fQ11) / (x2 - x1);
    }
    else if (y == y2)
    {
      return (fQ22 - fQ12) / (x2 - x1);
    }
    else if ((lx == ux) && lx > 0 && ux < _x1.size() - 1)
    {
      // expand grid size so x1 does not equal x2
      const auto & fQ01 = _zSurface(ly, lx - 1); // new lx at ly
      const auto & fQ31 = _zSurface(ly, lx + 1); // new ux at ly
      const auto & fQ02 = _zSurface(uy, lx - 1); // new lx at uy
      const auto & fQ32 = _zSurface(uy, lx + 1); // new ux at uy

      const Real & x0 = _x1[lx - 1]; // lx value
      const Real & x3 = _x1[lx + 1]; // ux value

      auto dfdx_a = fQ01 * (y - y2);
      dfdx_a += fQ11 * (y2 - y);
      dfdx_a += fQ02 * (y1 - y);
      dfdx_a += fQ12 * (y - y1);
      dfdx_a /= ((x1 - x0) * (y2 - y1));

      auto dfdx_b = fQ11 * (y - y2);
      dfdx_b += fQ31 * (y2 - y);
      dfdx_b += fQ12 * (y1 - y);
      dfdx_b += fQ32 * (y - y1);
      dfdx_b /= ((x3 - x1) * (y2 - y1));
      return 0.5 * (dfdx_a + dfdx_b);
    }
    else if ((lx == ux) && lx == 0)
    {
      const auto & fQ31 = _zSurface(ly, lx + 1); // new ux at ly
      const auto & fQ32 = _zSurface(uy, lx + 1); // new ux at uy

      const Real & x3 = _x1[lx + 1]; // ux value

      auto dfdx = fQ11 * (y - y2);
      dfdx += fQ31 * (y2 - y);
      dfdx += fQ12 * (y1 - y);
      dfdx += fQ32 * (y - y1);
      dfdx /= ((x3 - x1) * (y2 - y1));
      return 0.5 * dfdx;
    }
    else if ((lx == ux) && (ux == 99))
    {
      const auto & fQ01 = _zSurface(ly, ux - 1); // new lx at ly
      const auto & fQ02 = _zSurface(uy, ux - 1); // new lx at uy
      const Real & x0 = _x1[ux - 1]; // lx value

      auto dfdx = fQ01 * (y - y2);
      dfdx += fQ11 * (y2 - y);
      dfdx += fQ02 * (y1 - y);
      dfdx += fQ12 * (y - y1);
      dfdx /= ((x1 - x0) * (y2 - y1));
      return 0.5 * dfdx;
    }
    // Derivative (w/ respect to x) for some point inside box
    else
    {
      auto dfdx_xy = fQ11 * (y - y2);
      dfdx_xy += fQ21 * (y2 - y);
      dfdx_xy += fQ12 * (y1 - y);
      dfdx_xy += fQ22 * (y - y1);
      dfdx_xy /= ((x2 - x1) * (y2 - y1));
      return dfdx_xy;
    }
  }

  else if (deriv_var == 2)
  {
    if (x == x1) //if x equal to x1 node
    {
      return (fQ12 - fQ11) / (y2 - y1);
    }
    else if (x == x2) // if x equal to x2 node
    {
      return (fQ22 - fQ21) / (y2 - y1);
    }
    else if ((ly == uy) && ly > 0 && uy < _x2.size() - 1)
    {
      // expand grid size so x1 does not equal x2
      const auto & fQ10 = _zSurface(ly - 1, lx); // new ly at lx
      const auto & fQ20 = _zSurface(ly - 1, ux); // new uy at lx
      const auto & fQ13 = _zSurface(ly + 1, lx); // new ly at ux
      const auto & fQ23 = _zSurface(ly + 1, ux); // new uy at ux
      const Real & y0 = _x2[ly - 1];
      const Real & y3 = _x2[ly + 1];

      auto dfdy_a = fQ10 * (x - x2);
      dfdy_a += fQ20 * (x1 - x);
      dfdy_a += fQ11 * (x2 - x);
      dfdy_a += fQ21 * (x - x1);
      dfdy_a /= ((x2 - x1) * (y1 - y0));

      auto dfdy_b = fQ11 * (x - x2);
      dfdy_b += fQ21 * (x1 - x);
      dfdy_b += fQ13 * (x2 - x);
      dfdy_b += fQ23 * (x - x1);
      dfdy_b /= ((x2 - x1) * (y3 - y1));
      return 0.5 * (dfdy_a + dfdy_b);
    }
    else if ((ly == uy) && ly == 0)
    {
      const auto & fQ13 = _zSurface(ly + 1, lx); // new uy at lx
      const auto & fQ23 = _zSurface(ly + 1, ux); // new uy at ux

      const Real & y3 = _x2[ly + 1]; // lx value

      auto dfdy = fQ11 * (x - x2);
      dfdy += fQ21 * (x1 - x);
      dfdy += fQ13 * (x2 - x);
      dfdy += fQ23 * (x - x1);
      dfdy /= ((x2 - x1) * (y3 - y1));
      return 0.5 * dfdy;
    }
    else if ((ly == uy) && (uy == 99))
    {
      const auto & fQ10 = _zSurface(ly - 1, lx); // new ly at lx
      const auto & fQ20 = _zSurface(ly - 1, ux); // new ly at ux
      const Real & y0 = _x2[ly - 1]; // lx value

      auto dfdy = fQ10 * (x - x2);
      dfdy += fQ20 * (x1 - x);
      dfdy += fQ11 * (x2 - x);
      dfdy += fQ21 * (x - x1);
      dfdy /= ((x2 - x1) * (y1 - y0));
      return 0.5 * dfdy;
    }
    else
    {
      //Derivative (w/ respect to y) for any point inside box
      auto dfdy_xy = fQ11 * (x - x2);
      dfdy_xy += fQ21 * (x1 - x);
      dfdy_xy += fQ12 * (x2 - x);
      dfdy_xy += fQ22 * (x - x1);
      dfdy_xy /= ((x2 - x1) * (y2 - y1));
      return dfdy_xy;
    }
  }
  else
    mooseError("deriv_var must equal 1 or 2");
}

void BilinearInterpolation::sampleValueAndDerivatives(Real s1, Real s2, Real & y, Real & dy_ds1, Real & dy_ds2) const
{
  y = sample(s1, s2);
  dy_ds1 = sampleDerivative(s1, s2, 1);
  dy_ds2 = sampleDerivative(s1, s2, 2);
}
