/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PFCRFF_ENERGY_DENSITY_H
#define PFCRFF_ENERGY_DENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCRFFEnergyDensity;

template <>
InputParameters validParams<PFCRFFEnergyDensity>();

class PFCRFFEnergyDensity : public AuxKernel
{
public:
  PFCRFFEnergyDensity(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  unsigned int _order;
  std::vector<const VariableValue *> _vals;

  Real _a;
  Real _b;
  Real _c;
  unsigned int _num_exp_terms;
  MooseEnum _log_approach;
  Real _tol;
};

#endif // PFCRFF_ENERGY_DENSITY_H
