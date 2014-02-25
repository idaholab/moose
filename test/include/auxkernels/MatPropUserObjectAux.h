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

#ifndef MATPROPUSEROBJECTAUX_H
#define MATPROPUSEROBJECTAUX_H

#include "AuxKernel.h"

//Forward Declarations
class MatPropUserObjectAux;
class MaterialPropertyUserObject;

template<>
InputParameters validParams<MatPropUserObjectAux>();

class MatPropUserObjectAux : public AuxKernel
{
public:
  MatPropUserObjectAux(const std::string & name, InputParameters parameters);

  virtual ~MatPropUserObjectAux() {}

protected:
  virtual Real computeValue();

  const MaterialPropertyUserObject & _mat_uo;
};

#endif // MATPROPUSEROBJECTAUX_H
