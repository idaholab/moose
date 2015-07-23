#ifndef CHPFCRFF_H
#define CHPFCRFF_H

#include "Kernel.h"

//Forward Declarations
class CHPFCRFF;

template<>
InputParameters validParams<CHPFCRFF>();

/**
 * This kernel calculates the main portion of the cahn-hilliard residual for the RFF form of the phase field crystal model
 */
class CHPFCRFF : public Kernel
{
public:
  CHPFCRFF(const InputParameters & parameters);
  CHPFCRFF(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _M;
  bool _has_MJac;
  const MaterialProperty<Real> * _DM;

  MooseEnum _log_approach;
  Real _tol;
  std::vector<unsigned int> _vals_var;
  std::vector<VariableGradient *> _grad_vals;
  unsigned int _n_exp_terms;
  Real _a;
  Real _b;
  Real _c;

  unsigned int _num_L;
};

#endif //CHPFCRFF_H
