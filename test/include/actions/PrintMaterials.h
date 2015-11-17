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

#ifndef PRINTMATERIALS_H
#define PRINTMATERIALS_H

#include "Action.h"

class PrintMaterials;

template<>
InputParameters validParams<PrintMaterials>();

class PrintMaterials : public Action
{
public:
  PrintMaterials(const InputParameters & params);

  virtual void act();
};

#endif // PRINTMATERIALS_H
