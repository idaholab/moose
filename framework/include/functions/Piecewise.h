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

#ifndef PIECEWISE_H
#define PIECEWISE_H

#include "Function.h"
#include "LinearInterpolation.h"

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class Piecewise : public Function
{
public:
  Piecewise(const std::string & name, InputParameters parameters);
  virtual ~Piecewise();

  virtual Real timeDerivative(Real t, const Point & p) = 0;

  virtual Real functionSize();
  virtual Real domain(int i);
  virtual Real range(int i);

protected:
  const Real _scale_factor;
  LinearInterpolation * _linear_interp;
  int _axis;
  bool _has_axis;
private:
  const std::string _data_file_name;
  bool parseNextLineReals( std::ifstream & ifs, std::vector<Real> & myvec);
  void parseRows( std::vector<Real> & x, std::vector<Real> & y );
  void parseColumns( std::vector<Real> & x, std::vector<Real> & y);

};

template<>
InputParameters validParams<Piecewise>();

#endif
