#ifndef MATGRADSQCOUPLED_H
#define MATGRADSQCOUPLED_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class MatGradSqCoupled;

template <>
InputParameters validParams<MatGradSqCoupled>();

/*
 * This kernel calculates the prefactor * nabla_psi term in A-C equation for phase field modeling of oxidation
 * prefactor * grad_psi * grad_psi, where psi is the electrical field variable.
 * prefactor = 0.5 * grad_permitivity(phi), described in [Materials] using [DerivativeParsedMaterials]
 */

class MatGradSqCoupled : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  MatGradSqCoupled(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const VariableGradient & _grad_elec;
  const unsigned int _elec_var;

  const MaterialProperty<Real> & _prefactor;
  const MaterialProperty<Real> & _dprefactor_dphi;
  std::vector<const MaterialProperty<Real> *> _dprefactor_darg;
};

#endif // MATGRADSQCOUPLED_H
