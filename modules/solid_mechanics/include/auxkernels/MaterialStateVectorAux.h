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

#ifndef MATERIALSTATEVECTORAUX_H
#define MATERIALSTATEVECTORAUX_H

#include "AuxKernel.h"

//Forward Declarations
class MaterialStateVectorAux;

template<>
InputParameters validParams<MaterialStateVectorAux>();

class MaterialStateVectorAux : public AuxKernel
{
public:

  MaterialStateVectorAux(const std::string & name, InputParameters parameters);

protected:

  virtual double computeValue();

  MaterialProperty<std::vector<double> > & _vpStatefulProperty;
  int _index;
//  Real _factor;
 // const double _offset;

};

#endif //MATERIALSTATEVECTORAUX_H
