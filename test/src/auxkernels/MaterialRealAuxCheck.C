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

#include "MaterialRealAuxCheck.h"

template<>
InputParameters validParams<MaterialRealAuxCheck>()
{
  InputParameters params = validParams<MaterialRealAux>();
  return params;
}

MaterialRealAuxCheck::MaterialRealAuxCheck(const std::string & name, InputParameters parameters):
    MaterialRealAux(name, parameters)
{
  std::string prop = getParam<std::string>("property");
  if (!hasBlockMaterialProperty(prop))
    mooseError("the required material property "+prop+" is not defined on all blocks for aux kernel "+name);
}
