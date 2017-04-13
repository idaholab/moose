/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALRANDOMIC_H
#define POLYCRYSTALRANDOMIC_H

#include "InitialCondition.h"

// Forward Declarations
class PolycrystalRandomIC;

template <>
InputParameters validParams<PolycrystalRandomIC>();

/**
 * Random initial condition for a polycrystalline material
 */
class PolycrystalRandomIC : public InitialCondition
{
public:
  PolycrystalRandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

private:
  const unsigned int _op_num;
  const unsigned int _op_index;
  const unsigned int _typ;
};

#endif // POLYCRYSTALRANDOMIC_H
