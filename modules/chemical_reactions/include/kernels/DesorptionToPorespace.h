#ifndef DESORPTIONTOPORESPACE
#define DESORPTIONTOPORESPACE

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionToPorespace;

template<>
InputParameters validParams<DesorptionToPorespace>();

/**
 * Mass flow rate of fluid from the matrix to porespace
 * Add this to variable's equation so fluid contained in the matrix will desorbe to the variable's porespace mass
 */
class DesorptionToPorespace : public Kernel
{
public:

  DesorptionToPorespace(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Variable that is the concentration of fluid adsorbed inside the rock matrix
  VariableValue * _conc_val;

  /// MOOSE's internal variable number corresponding to _conc_val.  This is necessary for OffDiagJacobian computation
  unsigned int _conc_var;

  /// reciprocal of desorption time constant (got from LangmuirMaterial, for instance)
  MaterialProperty<Real> &_one_over_desorption_time_const;

  /// reciprocal of adsorption time constant (got from LangmuirMaterial, for instance)
  MaterialProperty<Real> &_one_over_adsorption_time_const;

  /// equilibrium concentration of the adsorbed fluid
  MaterialProperty<Real> &_equilib_conc;

  /// derivative of equilibrium concentration of the adsorbed fluid wrt the porepressure
  MaterialProperty<Real> &_equilib_conc_prime;
};

#endif //DESORPTIONTOPORESPACE
