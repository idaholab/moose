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

#ifndef BILINEARINTERPOLATION_H
#define BILINEARINTERPOLATION_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "ColumnMajorMatrix.h"


/**
 * This class applies the Least Squares algorithm to a set of points to provide a smooth curve for
 * sampling values.
 *
 * Requires: LAPACK
 */
class BilinearInterpolation
{
public:

  /* Constructor, Takes two vectors of points for which to apply the fit.  One should be of the
   * independent variable while the other should be of the dependent varible.  These values should
   * correspond to one and other in the same position.
   */
  BilinearInterpolation(const std::vector<Real> & XAXIS,
                        const std::vector<Real> & YAXIS,
                        const ColumnMajorMatrix  & ZSURFACE);

  virtual ~BilinearInterpolation()
    {}

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on the generated fit
   */
  Real sample(Real xcoord, Real ycoord);

  void GetNeigbourIndices(std::vector<Real> inArr, Real x ,int& lowerX ,int& upperX );
  
private:

  std::vector<Real> _xAxis;
  std::vector<Real> _yAxis;
  ColumnMajorMatrix _zSurface;
  static int _file_number;
};

#endif //BILINEARINTERPOLATION_H
  
  
  
      
