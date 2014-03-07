#ifndef DESORPTIONFROMMATRIX
#define DESORPTIONFROMMATRIX

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionFromMatrix;

template<>
InputParameters validParams<DesorptionFromMatrix>();

class DesorptionFromMatrix : public Kernel
{
public:

  DesorptionFromMatrix(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _pressure_var;

  MaterialProperty<Real> &_desorption_time_const;
  MaterialProperty<Real> &_adsorption_time_const;
  MaterialProperty<Real> &_equilib_conc;
  MaterialProperty<Real> &_equilib_conc_prime;

};

#endif //DESORPTIONFROMMATRIX
