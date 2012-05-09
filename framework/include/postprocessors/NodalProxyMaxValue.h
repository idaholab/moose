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

#ifndef NODALPROXYMAXVALUE_H
#define NODALPROXYMAXVALUE_H

#include "NodalPostprocessor.h"

//Forward Declarations
class NodalProxyMaxValue;
class MooseMesh;

template<>
InputParameters validParams<NodalProxyMaxValue>();

class NodalProxyMaxValue : public NodalPostprocessor
{
public:
  NodalProxyMaxValue(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

  void threadJoin(const Postprocessor & y);

protected:

  Real _value;
  unsigned int _node_id;
};

#endif //NODALPROXYMAXVALUE_H
