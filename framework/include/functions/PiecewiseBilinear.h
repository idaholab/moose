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

#ifndef PIECEWISEBILINEAR_H
#define PIECEWISEBILINEAR_H

#include "Function.h"
#include "BilinearInterpolation.h"
#include "ColumnMajorMatrix.h"

class PiecewiseBilinear;

template<>
InputParameters validParams<PiecewiseBilinear>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class PiecewiseBilinear : public Function
{
public:
  PiecewiseBilinear(const std::string & name, InputParameters parameters);
  virtual ~PiecewiseBilinear();

  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real value(Real t, const Point & pt);



private:
  BilinearInterpolation * _bilinear_interp;
  const std::string _file_name;
  
//  Real _scale_factor;


  void parse( std::vector<Real> & x,
              std::vector<Real> & y,
              ColumnMajorMatrix & z);
};

#endif //PIECEWISEBILINEAR_H
