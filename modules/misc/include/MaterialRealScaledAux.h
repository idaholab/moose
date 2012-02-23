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

#ifndef MATERIALREALSCALEDAUX_H
#define MATERIALREALSCALEDAUX_H

#include "AuxKernel.h"

//Forward Declarations
class MaterialRealScaledAux;

template<>
InputParameters validParams<MaterialRealScaledAux>();

class MaterialRealScaledAux : public AuxKernel
{
public:

  MaterialRealScaledAux(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeValue();
  std::string _matpro;
  
private:

  MaterialProperty<Real> & _prop;
  Real _factor;
  Real _offset;

};

#endif //MATERIALREALSCALEDAUX_H
