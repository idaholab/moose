/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CINTERFACEPOSITION_H
#define CINTERFACEPOSITION_H

#include "NodalProxyMaxValue.h"

// Forward Declarations
class CInterfacePosition;
class MooseMesh;

template <>
InputParameters validParams<CInterfacePosition>();

class CInterfacePosition : public NodalProxyMaxValue
{
public:
  CInterfacePosition(const InputParameters & parameters);
  virtual Real getValue();

protected:
  virtual Real computeValue();

  Real _RefVal;
  unsigned int _direction_index;
  MooseMesh & _mesh;
};

#endif // CINTERFACEPOSITION_H
