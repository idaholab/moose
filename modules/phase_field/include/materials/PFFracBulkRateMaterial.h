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

#ifndef PFFRACBULKRATEMATERIAL_H
#define PFFRACBULKRATEMATERIAL_H

#include "Material.h"
#include "Function.h"

/**
 * Phase-field fracture
 * This class obtains critical energy release rate (gc) value
 * Used by PFFRacBulkRate
 */

class PFFracBulkRateMaterial;

template<>
InputParameters validParams<PFFracBulkRateMaterial>();

class PFFracBulkRateMaterial : public Material
{
public:
  PFFracBulkRateMaterial(const std::string & name,
                        InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  /**
   * This function obtains the value of gc
   * Must be overidden by the user for heterogeneous gc
   */
  virtual void getProp(){}

  ///Input parameter for homogeneous gc
  Real _gc;

  ///Material property where the values are stored
  MaterialProperty<Real> &_gc_prop;

private:

};

#endif //PFFRACBULKRATEMATERIAL_H
