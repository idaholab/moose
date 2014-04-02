#ifndef DESORPTIONFROMMATRIX
#define DESORPTIONFROMMATRIX

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionFromMatrix;

template<>
InputParameters validParams<DesorptionFromMatrix>();

/**
 * Mass flow rate of adsorbed fluid from matrix
 * Add this to TimeDerivative to form the entire DE for desorption of fluid-in-the-matrix
 */
class DesorptionFromMatrix : public Kernel
{
public:

  DesorptionFromMatrix(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// MOOSE's internal variable number corresonding to the porepressure - need this for OffDiagJacobian calculation
  unsigned int _pressure_var;

  /// reciprocal of desorption time constant (got from LangmuirMaterial, for instance)
  MaterialProperty<Real> &_one_over_desorption_time_const;

  /// reciprocal of adsorption time constant (got from LangmuirMaterial, for instance)
  MaterialProperty<Real> &_one_over_adsorption_time_const;

  /// equilibrium concentration of the adsorbed fluid
  MaterialProperty<Real> &_equilib_conc;

  /// derivative of equilibrium concentration of the adsorbed fluid wrt the porepressure
  MaterialProperty<Real> &_equilib_conc_prime;

};

#endif //DESORPTIONFROMMATRIX
