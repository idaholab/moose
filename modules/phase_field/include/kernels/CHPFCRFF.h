/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CHPFCRFF_H
#define CHPFCRFF_H

#include "Kernel.h"

// Forward Declarations
class CHPFCRFF;

template <>
InputParameters validParams<CHPFCRFF>();

/**
 * This kernel calculates the main portion of the cahn-hilliard residual for the
 * RFF form of the phase field crystal model
 */
class CHPFCRFF : public Kernel
{
public:
  CHPFCRFF(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _M;
  const bool _has_MJac;
  const MaterialProperty<Real> * _DM;

  const MooseEnum _log_approach;
  const Real _tol;

  const unsigned int _num_L;
  std::vector<unsigned int> _vals_var;
  std::vector<const VariableGradient *> _grad_vals;

  const unsigned int _n_exp_terms;
  const Real _a;
  const Real _b;
  const Real _c;
};

#endif // CHPFCRFF_H
