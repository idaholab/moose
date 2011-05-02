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
#include "libmesh_common.h"
#include "Moose.h"

int BilinearInterpolation::_file_number = 0;

BilinearInterpolation::BilinearInterpolation(const std::vector<Real> & x, const std::vector<Real> & y, const ColumnMajorMatrix & z): _xAxis(x), _yAxis(y), _zSurface(z)
{
}

void BilinearInterpolation::GetNeigbourIndices(std::vector<Real> inArr, Real x ,int& lowerX ,int& upperX )
{
  int N = inArr.size();
	if (x <= inArr[0])
	{
		lowerX = 1;
		upperX = 1;
	}
	else if (x >= inArr[N-1] )
	{
		lowerX = N;
		upperX = N;
	}
	else
	{
		for (int i = 2; i<=N; i++)
		{
                  if (x < inArr[i-1] )
			{
				lowerX = i - 1;
				upperX = i;
				break;
			}
                  else if (x == inArr[i-1])
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
	//first find 4 neighbouring points
	//int nx = xAxis->size;
	//int ny = yAxis->size;
	int lx; //index of x coordinate of adjacent grid point to left of P
	int ux; //index of x coordinate of adjacent grid point to right of P
	GetNeigbourIndices( _xAxis, xcoord, lx, ux);
	int ly; //index of y coordinate of adjacent grid point below P
	int uy; //index of y coordinate of adjacent grid point above P
	GetNeigbourIndices( _yAxis, ycoord, ly, uy);
/*	Real fQ11 = _zSurface(lx-1, ly-1);
	Real fQ21 = _zSurface(ux-1, ly-1);
	Real fQ12 = _zSurface(lx-1, uy-1);
	Real fQ22 = _zSurface(ux-1, uy-1);
*/
	Real fQ11 = _zSurface(ly-1, lx-1);
	Real fQ21 = _zSurface(ly-1, ux-1);
	Real fQ12 = _zSurface(uy-1, lx-1);
	Real fQ22 = _zSurface(uy-1, ux-1);
	//if point exactly found on a node do not interpolate
	if ((lx == ux) && (ly == uy))  
		return fQ11;
	Real x = xcoord;
	Real y = ycoord;
	Real x1 = _xAxis[lx-1];
	Real x2 = _xAxis[ux-1];
	Real y1 = _yAxis[ly-1];
	Real y2 = _yAxis[uy-1];
	//if xcoord lies exactly on an xAxis node do linear interpolation
	if (lx == ux) 
		return fQ11 + (fQ12 - fQ11) * (y - y1) / (y2 - y1);
	//if ycoord lies exactly on an xAxis node do linear interpolation
	if (ly == uy) 
		return fQ11 + (fQ22 - fQ11) * (x - x1) / (x2 - x1);
	Real fxy = fQ11 * (x2 - x) * (y2 - y);
	fxy = fxy + fQ21 * (x - x1) * (y2 - y);
	fxy = fxy + fQ12 * (x2 - x) * (y - y1);
	fxy = fxy + fQ22 * (x - x1) * (y - y1);
	fxy = fxy / ((x2 - x1) * (y2 - y1));
/*  std::cout << _xAxis[0] << "xAxis0 from sample" << std::endl;
  std::cout << xcoord << "xcoord from sample" << std::endl;
  std::cout << ycoord << "ycoord from sample" << std::endl;
  std::cout << _zSurface(0,0) << "z(0,0) from sample" << std::endl;
  std::cout << _zSurface(0,1) << "z(0,1) from sample" << std::endl;
  std::cout << _zSurface(1,0) << "z(1,0) from sample" << std::endl;
  std::cout << _zSurface(1,1) << "z(0,0) from sample" << std::endl;
*/  
	return fxy;
        return 0;
}

