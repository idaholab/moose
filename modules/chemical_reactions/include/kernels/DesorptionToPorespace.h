#ifndef DESORPTIONTOPORESPACE
#define DESORPTIONTOPORESPACE

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionToPorespace;

template<>
InputParameters validParams<DesorptionToPorespace>();

class DesorptionToPorespace : public Kernel
{
public:

  DesorptionToPorespace(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue * _conc_val;
  unsigned int _conc_var;

  MaterialProperty<Real> &_desorption_time_const;
  MaterialProperty<Real> &_adsorption_time_const;
  MaterialProperty<Real> &_equilib_conc;
  MaterialProperty<Real> &_equilib_conc_prime;

};

#endif //DESORPTIONTOPORESPACE
