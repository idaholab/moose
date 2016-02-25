/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KINETICDISPRERATEAUX_H
#define KINETICDISPRERATEAUX_H

#include "AuxKernel.h"

//Forward Declarations
class KineticDisPreRateAux;

template<>
InputParameters validParams<KineticDisPreRateAux>();

/**
 * Coupled auxiliary value
 */
class KineticDisPreRateAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  KineticDisPreRateAux(const InputParameters & parameters);

  virtual ~KineticDisPreRateAux() {}

protected:
  virtual Real computeValue();

  Real _log_k,_r_area,_ref_kconst,_e_act,_gas_const,_ref_temp,_sys_temp;
  std::vector<Real> _sto_v;

  std::vector<const VariableValue *> _vals;
};

#endif //KINETICDISPRERATEAUX_H
