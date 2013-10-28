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

/* BilinearInterpolation is designed to linearly interpolate a function of
 * two values e.g. z(x,y).  Supply Bilinearlinear with a vector of x and a
 * vector of y and a ColumnMajorMatrix of function values, z, that correspond
 * to the values in the vectors x and y...and also a sample point
 * (xcoord and ycoord), and BilinearInterpolation will return the value of the
 * function at the sample point.  A simple example:
 *
 * x = [1 2], y = [1 2],
 *
 * z = [1 2]
 *     [3 4]
 *
 * with xcoord = 1.5 and ycoord = 1.5 returns a value of 2.5
 */

#include "BilinearInterpolation.h"
#include "libmesh/libmesh_common.h"

int BilinearInterpolation::_file_number = 0;

BilinearInterpolation::BilinearInterpolation(const std::vector<Real> & x, const std::vector<Real> & y, const ColumnMajorMatrix & z): _xAxis(x), _yAxis(y), _zSurface(z)
{
}

void BilinearInterpolation::getNeighborIndices(std::vector<Real> inArr, Real x ,int& lowerX ,int& upperX )
{
  int N = inArr.size();
	if (x <= inArr[0])
	{
		lowerX = 0;
		upperX = 0;
	}
	else if (x >= inArr[N-1] )
	{
		lowerX = N-1;
		upperX = N-1;
	}
	else
	{
		for (int i(1); i < N; ++i)
		{
      if (x < inArr[i] )
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

Real BilinearInterpolation::sample(Real xcoord, Real ycoord)
{
	//first find 4 neighboring points
	int lx=0; //index of x coordinate of adjacent grid point to left of P
	int ux=0; //index of x coordinate of adjacent grid point to right of P
	getNeighborIndices( _xAxis, xcoord, lx, ux);
	int ly=0; //index of y coordinate of adjacent grid point below P
	int uy=0; //index of y coordinate of adjacent grid point above P
	getNeighborIndices( _yAxis, ycoord, ly, uy);
	Real fQ11 = _zSurface(ly, lx);
	Real fQ21 = _zSurface(ly, ux);
	Real fQ12 = _zSurface(uy, lx);
	Real fQ22 = _zSurface(uy, ux);
	//if point exactly found on a node do not interpolate
	if ((lx == ux) && (ly == uy))
		return fQ11;
	Real x = xcoord;
	Real y = ycoord;
	Real x1 = _xAxis[lx];
	Real x2 = _xAxis[ux];
	Real y1 = _yAxis[ly];
	Real y2 = _yAxis[uy];
	//if xcoord lies exactly on an xAxis node do linear interpolation
	if (lx == ux)
	{
		return fQ11 + (fQ12 - fQ11) * (y - y1) / (y2 - y1);
	}
	//if ycoord lies exactly on an yAxis node do linear interpolation
	if (ly == uy)
	{
		return fQ11 + (fQ21 - fQ11) * (x - x1) / (x2 - x1);
	}
	Real fxy = fQ11 * (x2 - x) * (y2 - y);
	fxy += fQ21 * (x - x1) * (y2 - y);
	fxy += fQ12 * (x2 - x) * (y - y1);
	fxy += fQ22 * (x - x1) * (y - y1);
	fxy /= ((x2 - x1) * (y2 - y1));

        return fxy;
}

