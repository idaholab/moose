/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATSOURCE_H
#define HEATSOURCE_H

#include "BodyForce.h"

//Forward Declarations
class HeatSource;

template<>
InputParameters validParams<HeatSource>();

class HeatSource : public BodyForce
{
public:
  HeatSource(const InputParameters & parameters);
};

#endif
