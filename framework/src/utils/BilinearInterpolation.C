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

  // std::cout << "x index" << lx << " " << ux << std::endl;
  // std::cout << "y index" << ly << " " << uy << std::endl;
  const auto & x = s1;
  const auto & y = s2;
  const Real & x1 = _x1[lx];
  const Real & x2 = _x1[ux];
  const Real & y1 = _x2[ly];
  const Real & y2 = _x2[uy];

  if ((lx == 0) && (ly == 0)) // if at bottom left node, take average slope of four boxes
  //  [IV][I]
  //  [III][II]
  {
    const auto & fQ00 = _zSurface(ly - 1, lx - 1); // fQ at (x0,y0)
    const auto & fQ01 = _zSurface(ly, lx - 1); // fQ at (x0,y1) note, y1=y2 if ly=uy
    const auto & fQ03 = _zSurface(ly + 1, lx - 1); // fQ at (x0,y3)

    const auto & fQ10 = _zSurface(ly - 1, lx); // fQ at (x1,y0)
    const auto & fQ13 = _zSurface(ly + 1, lx); // fQ at (x1,y3)

    const auto & fQ30 = _zSurface(ly - 1, lx + 1); //fQ at (x3,y0)
    const auto & fQ31 = _zSurface(ly, lx + 1); //fQ at (x3,y1)
    const auto & fQ33 = _zSurface(ly + 1, lx + 1); //fQ at (x3,y3)

    const Real & x0 = _x1[lx - 1]; // lx value
    const Real & y0 = _x2[ly - 1]; // ly value
    const Real & x3 = _x1[lx + 1]; // ux value
    const Real & y3 = _x2[ly + 1]; // uy value

    if (deriv_var == 1)
    {
      auto dfdx1 = fQ11 * (y - y3);
      dfdx1 += fQ31 * (y3 - y);
      dfdx1 += fQ13 * (y1 - y);
      dfdx1 += fQ33 * (y - y1);
      dfdx1 /= ((x3 - x1) * (y3 - y1));
      //
      auto dfdx2 = fQ10 * (y - y1);
      dfdx2 += fQ30 * (y1 - y);
      dfdx2 += fQ11 * (y0 - y);
      dfdx2 += fQ31 * (y - y0);
      dfdx2 /= ((x3 - x1) * (y1 - y0));

      auto dfdx3 = fQ00 * (y - y1);
      dfdx3 += fQ10 * (y1 - y);
      dfdx3 += fQ01 * (y0 - y);
      dfdx3 += fQ11 * (y - y0);
      dfdx3 /= ((x1 - x0) * (y1 - y0));

      auto dfdx4 = fQ01 * (y - y3);
      dfdx4 += fQ11 * (y3 - y);
      dfdx4 += fQ03 * (y1 - y);
      dfdx4 += fQ13 * (y - y1);
      dfdx4 /= ((x1 - x0) * (y3 - y1));
      std::cout << "at bottom left node, dfdx = " << 0.25 * (dfdx1 + dfdx2 + dfdx3 + dfdx4) << std::endl;
      return 0.25 * (dfdx1 + dfdx2 + dfdx3 + dfdx4);
    }
    else if (deriv_var == 2)
    {
      auto dfdy1 = fQ11 * (x - x3);
      dfdy1 += fQ31 * (x1 - x);
      dfdy1 += fQ13 * (x3 - x);
      dfdy1 += fQ33 * (x - x1);
      dfdy1 /= ((x3 - x1) * (y3 - y1));

      auto dfdy2 = fQ10 * (x - x3);
      dfdy2 += fQ30 * (x1 - x);
      dfdy2 += fQ11 * (x3 - x);
      dfdy2 += fQ31 * (x - x1);
      dfdy2 /= ((x3 - x1) * (y1 - y0));

      auto dfdy3 = fQ00 * (x - x1);
      dfdy3 += fQ10 * (x0 - x);
      dfdy3 += fQ01 * (x1 - x);
      dfdy3 += fQ11 * (x - x0);
      dfdy3 /= ((x1 - x0) * (y1 - y0));

      auto dfdy4 = fQ01 * (x - x1);
      dfdy4 += fQ11 * (x0 - x);
      dfdy4 += fQ03 * (x1 - x);
      dfdy4 += fQ13 * (x - x0);
      dfdy4 /= ((x1 - x0) * (y3 - y1));
      std::cout << "at bottom left node, dfdy = " << 0.25 * (dfdy1 + dfdy2 + dfdy3 + dfdy4) << std::endl;
      return 0.25 * (dfdy1 + dfdy2 + dfdy3 + dfdy4);
    }
    else
      mooseError("deriv_var must equal 1 or 2");
  }
  else if ((ly == 0) && (ux == 99)) // if at bottom right node
  {
    mooseWarning("Derivative sampled at bottom right node, returning 0 as derivative");
    return 0;
  }
  else if ((uy == 99) && (lx == 0)) // if at top left node
  {
    mooseWarning("Derivative sampled at top left node, returning 0 as derivative");
    return 0;
  }
  else if ((uy == 99) && (ux == 99)) // if at top right node
  {
    mooseWarning("Derivative sampled at top right node, returning 0 as derivative");
    return 0;
  }
  // calculate derivative wrt to x
  if (deriv_var == 1)
  {
    // Find derivative when on interval between two nodes
    if (y == y1)
    {
      // std::cout << "y==ly" <<std::endl;
      auto dfdx_ly = (fQ21 - fQ11) / (x2 - x1);
      return dfdx_ly;
    }
    else if (y == y2) //
    {
      // std::cout << "y==uy" <<std::endl;
      auto dfdx_uy = (fQ22 - fQ12) / (x2 - x1);
      // std::cout << "fQ22 - fQ12 = " << fQ22 - fQ12 << std::endl;
      // std::cout << "x2 - x1 = " << x2 - x1 << std::endl;
      return dfdx_uy;
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
      // std::cout << "expanded grid occurs, derivative = " << 0.5 * (dfdx_a + dfdx_b) << std::endl;
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
      // std::cout << "lx = 0, dfdx =  " << 0.5 * dfdx << std::endl;
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
      // std::cout << "ux = 99, dfdx =  " << 0.5 * dfdx << std::endl;
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
      // std::cout << "dfdx_xy = " << dfdx_xy << std::endl;
      return dfdx_xy;
    }
  }

  else if (deriv_var == 2)
  {
    if (x == x1) //if x equal to x1 node
    {
      // std::cout << "x==lx" <<std::endl;
      auto dfdy_lx = (fQ12 - fQ11) / (y2 - y1);
      // std::cout << "fQ12 - fQ11 = " << fQ12 - fQ11 << std::endl;
      // std::cout << "y2 - y1 = " << y2 - y1 << std::endl;
      return dfdy_lx;
    }
    else if (x == x2) // if x equal to x2 node
    {
      // std::cout << "x==ux" <<std::endl;
      auto dfdy_ux = (fQ22 - fQ21) / (y2 - y1);
      return dfdy_ux;
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
      // std::cout << "expanded grid occurs, derivative = " << 0.5 * (dfdy_a + dfdy_b) << std::endl;
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
      // std::cout << "ly = 0, dfdy =  " << 0.5 * dfdy << std::endl;
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
      // std::cout << "uy = 99, dfdy =  " << 0.5 * dfdy << std::endl;
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
      // std::cout << "dfdy_xy = " << dfdy_xy << std::endl;
      return dfdy_xy;
    }
  }
  else
    mooseError("deriv_var must equal 1 or 2");
}

void BilinearInterpolation::sampleValueAndDerivatives(Real s1, Real s2, Real & y, Real & dy_ds1, Real & dy_ds2) const
{
  y = sample(s1, s2);
  // std::cout << "wrt x1 " << std::endl;
  dy_ds1 = sampleDerivative(s1, s2, 1);
  // std::cout << "wrt x2 " << std::endl;
  dy_ds2 = sampleDerivative(s1, s2, 2);
  // std::cout << "sample " << " " << y << std::endl;
  // std::cout << "sampleValueAndDerivatives " << " " << dy_dx1 << " " << dy_dx2 << std::endl;
}
