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

#ifndef CINTERFACEPOSITION_H
#define CINTERFACEPOSITION_H

#include "NodalProxyMaxValue.h"

//Forward Declarations
class CInterfacePosition;
class MooseMesh;

template<>
InputParameters validParams<CInterfacePosition>();

class CInterfacePosition : public NodalProxyMaxValue
{
public:
  CInterfacePosition(const std::string & name, InputParameters parameters);
  virtual Real getValue();

protected:
  virtual Real computeValue();

  Real _RefVal;
  unsigned int _direction_index;
  MooseMesh & _mesh;

};

#endif //CINTERFACEPOSITION_H
