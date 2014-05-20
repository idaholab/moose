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

#ifndef MATERIALREALAUXCHECK_H
#define MATERIALREALAUXCHECK_H

// MOOSE includes
#include "MaterialRealAux.h"

//Forward Declarations
class MaterialRealAuxCheck;

template<>
InputParameters validParams<MaterialRealAuxCheck>();

/**
 * Object for passing a scalar, REAL material property to an AuxVariable
 */
class MaterialRealAuxCheck : public MaterialRealAux
{
public:

  /**
   * Class constructor.
   * @param name Name of the object
   * @param parameters Input parameters for this object
   */
  MaterialRealAuxCheck(const std::string & name, InputParameters parameters);
};

#endif //MATERIALREALAUXCHECK_H
