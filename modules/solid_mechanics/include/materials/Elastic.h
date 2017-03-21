/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELASTIC_H
#define ELASTIC_H

#include "SolidModel.h"

class Elastic : public SolidModel
{
public:
  Elastic(const InputParameters & parameters);
  virtual ~Elastic();

protected:
};

template <>
InputParameters validParams<Elastic>();

#endif
