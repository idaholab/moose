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

#ifndef POINTVALUE_H
#define POINTVALUE_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PointValue;

template<>
InputParameters validParams<PointValue>();

class PointValue : public GeneralPostprocessor
{
public:
  PointValue(const std::string & name, InputParameters parameters);
  virtual ~PointValue();
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  MooseVariable & _var;
  VariableValue & _u;
  MooseMesh & _mesh;
  Point _point;
  std::vector<Point> _point_vec;
  Real _value;
};



#endif /* POINTVALUE_H */
