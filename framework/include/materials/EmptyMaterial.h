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

#ifndef EMPTYMATERIAL_H
#define EMPTYMATERIAL_H

#include "Material.h"


//Forward Declarations
class EmptyMaterial;

template<>
InputParameters validParams<EmptyMaterial>();

/**
 * Empty material for use in simple applications that don't need material properties.
 */
class EmptyMaterial : public Material
{
public:
  EmptyMaterial(std::string name,
                MooseSystem & moose_system,
                InputParameters parameters);
  
protected:
  virtual void computeProperties();
};

#endif //EMPTYMATERIAL_H
