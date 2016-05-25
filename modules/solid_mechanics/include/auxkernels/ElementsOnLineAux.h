/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELEMENTSONLINEAUX_H
#define ELEMENTSONLINEAUX_H

#include "AuxKernel.h"

class ElementsOnLineAux : public AuxKernel
{

public:

  ElementsOnLineAux(const InputParameters & parameters);

  virtual ~ElementsOnLineAux() {}

protected:

  virtual void compute();
  virtual Real computeValue();

private:

  const Point _line1;
  const Point _line2;
  const Real _dist_tol;

  int _line_id;

};

template<>
InputParameters validParams<ElementsOnLineAux>();

#endif // ELEMENTSONLINEAUX_H

