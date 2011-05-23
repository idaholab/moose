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

#ifndef MATERIALREALAUX_H
#define MATERIALREALAUX_H

#include "AuxKernel.h"

//Forward Declarations
class MaterialRealAux;

template<>
InputParameters validParams<MaterialRealAux>();

class MaterialRealAux : public AuxKernel
{
public:
  
  MaterialRealAux(const std::string & name, InputParameters parameters);
  
protected:

  virtual Real computeValue();
  std::string _matpro;
 
private:
  
  MaterialProperty<Real> & _prop;
  
};

#endif //MATERIALREALAUX_H
