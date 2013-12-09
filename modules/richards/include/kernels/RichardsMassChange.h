#ifndef RICHARDSMASSCHANGE
#define RICHARDSMASSCHANGE

#include "Kernel.h"

// Forward Declarations
class RichardsMassChange;

template<>
InputParameters validParams<RichardsMassChange>();

class RichardsMassChange : public Kernel
{
public:

  RichardsMassChange(const std::string & name,
                        InputParameters parameters);

  //virtual void computeJacobian();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  bool _lumping;
  bool _use_supg;

  MaterialProperty<Real> &_porosity;

  MaterialProperty<Real> &_sat_old;

  MaterialProperty<Real> &_sat;
  MaterialProperty<Real> &_dsat;
  MaterialProperty<Real> &_d2sat;

  MaterialProperty<Real> &_density_old;

  MaterialProperty<Real> &_density;
  MaterialProperty<Real> &_ddensity;
  MaterialProperty<Real> &_d2density;

  MaterialProperty<RealVectorValue> &_vel_SUPG;
  MaterialProperty<RealTensorValue> &_vel_prime_SUPG;
  MaterialProperty<Real> &_tau_SUPG;
  MaterialProperty<RealVectorValue> &_tau_prime_SUPG;
};

#endif //RICHARDSMASSCHANGE
