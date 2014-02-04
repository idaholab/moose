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

#include "Material.h"

#ifndef BADSTATEFULMATERIAL_H
#define BADSTATEFULMATERIAL_H

//Forward Declarations
class BadStatefulMaterial;

template<>
InputParameters validParams<BadStatefulMaterial>();

// Only declares old or older property
class BadStatefulMaterial : public Material
{
public:
  BadStatefulMaterial(const std::string & name,
                      InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

private:
  MaterialProperty<Real> & _prop_old;
};

#endif //STATEFULMATERIAL_H
