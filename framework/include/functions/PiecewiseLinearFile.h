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

#ifndef PIECEWISELINEARFILE_H
#define PIECEWISELINEARFILE_H

#include "Function.h"
#include "LinearInterpolation.h"

class PiecewiseLinearFile;

template<>
InputParameters validParams<PiecewiseLinearFile>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class PiecewiseLinearFile : public Function
{
public:
  PiecewiseLinearFile(const std::string & name, InputParameters parameters);
  virtual ~PiecewiseLinearFile();

  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real value(Real t, const Point & pt);

  virtual Real integral();
  virtual Real average();

private:
  LinearInterpolation * _linear_interp;
  const std::string _file_name;
  Real _scale_factor;
  const std::string _format;
  void parseRows( std::vector<Real> & x, std::vector<Real> & y );
  void parseColumns( std::vector<Real> & x, std::vector<Real> & y);

};

#endif //PIECEWISELINEARFILE_H
