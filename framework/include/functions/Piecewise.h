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

class Piecewise;

template<>
InputParameters validParams<Piecewise>();

/**
 * Function which provides a piecewise approximation to a provided
 * (x,y) point data set.  Derived classes which control the order
 * (constant, linear) of the approximation should be used directly.
 */
class Piecewise : public Function
{
public:
  Piecewise(const InputParameters & parameters);

  virtual Real functionSize();
  virtual Real domain(int i);
  virtual Real range(int i);

protected:
  const Real _scale_factor;
  std::unique_ptr<LinearInterpolation> _linear_interp;
  int _axis;
  bool _has_axis;
private:
  const std::string _data_file_name;
  unsigned int _x_index;
  unsigned int _y_index;
  bool _xy_only;
  bool parseNextLineReals( std::ifstream & ifs, std::vector<Real> & myvec);
  void parseRows( std::vector<Real> & x, std::vector<Real> & y );
  void parseColumns( std::vector<Real> & x, std::vector<Real> & y);
};

#endif
