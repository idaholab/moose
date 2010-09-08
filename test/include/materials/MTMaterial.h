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

#ifndef MTMATERIAL_H
#define MTMATERIAL_H

#include "Material.h"


//Forward Declarations
class MTMaterial;

template<>
InputParameters validParams<MTMaterial>();

/**
 * Simple material with constant properties.
 */
class MTMaterial : public Material
{
public:
  MTMaterial(const std::string & name,
             MooseSystem & moose_system,
             InputParameters parameters);
  
protected:
  virtual void computeProperties();
  
private:
  MaterialProperty<Real> & _mat_prop;
};

#endif //DIFF1MATERIAL_H
