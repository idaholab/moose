#ifndef SWITCHINGFUNCTIONCONSTRAINTLAGRANGE_H
#define SWITCHINGFUNCTIONCONSTRAINTLAGRANGE_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class SwitchingFunctionConstraintLagrange;

template<>
InputParameters validParams<SwitchingFunctionConstraintLagrange>();

/**
 * SwitchingFunctionConstraintLagrange is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class SwitchingFunctionConstraintLagrange : public DerivativeMaterialInterface<Kernel>
{
public:
  SwitchingFunctionConstraintLagrange(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function names
  std::vector<std::string> _h_names;
  unsigned int _num_h;

  /// Switching functions and their drivatives
  std::vector<const MaterialProperty<Real> *> _h, _dh;

  /// eta index for the j_vars in the jacobian computation
  LIBMESH_BEST_UNORDERED_MAP<unsigned int, unsigned int> _j_eta;
};

#endif //SWITCHINGFUNCTIONCONSTRAINTLAGRANGE_H
