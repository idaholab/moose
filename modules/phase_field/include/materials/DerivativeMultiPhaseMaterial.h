/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVEMULTIPHASEMATERIAL_H
#define DERIVATIVEMULTIPHASEMATERIAL_H

#include "DerivativeMultiPhaseBase.h"

class DerivativeMultiPhaseMaterial;

template <>
InputParameters validParams<DerivativeMultiPhaseMaterial>();

/**
 * Multi phase free energy material that combines an arbitrary number of
 * phase free energies to a global free energy. All switching functions are
 * assumed to depend only on their respective order parameter.
 */
class DerivativeMultiPhaseMaterial : public DerivativeMultiPhaseBase
{
public:
  DerivativeMultiPhaseMaterial(const InputParameters & parameters);

protected:
  virtual Real computeDF(unsigned int i_var);
  virtual Real computeD2F(unsigned int i_var, unsigned int j_var);
  virtual Real computeD3F(unsigned int i_var, unsigned int j_var, unsigned int k_var);

  /// Function value of the i phase.
  std::vector<const MaterialProperty<Real> *> _dhi, _d2hi, _d3hi;
};

#endif // DERIVATIVEMULTIPHASEMATERIAL_H
