//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

template <>
InputParameters validParams<PFFracBulkRateMaterial>();

class PFFracBulkRateMaterial : public Material
{
public:
  PFFracBulkRateMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  /**
   * This function obtains the value of gc
   * Must be overidden by the user for heterogeneous gc
   */
  virtual void getProp();

  ///Input parameter for homogeneous gc
  Real _gc;

  ///Material property where the gc values are stored
  MaterialProperty<Real> & _gc_prop;
  ///Function to specify varying gc
  Function * _function_prop;

private:
};

#endif // PFFRACBULKRATEMATERIAL_H
