#ifndef RICHARDSFLUXJUMPINDICATOR_H
#define RICHARDSFLUXJUMPINDICATOR_H

#include "JumpIndicator.h"

class RichardsFluxJumpIndicator;

template<>
InputParameters validParams<RichardsFluxJumpIndicator>();

class RichardsFluxJumpIndicator :
  public JumpIndicator
{
public:
  RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters);
  virtual ~RichardsFluxJumpIndicator(){};

protected:

  virtual Real computeQpIntegral();

  MaterialProperty<Real> &_density;
  MaterialProperty<Real> &_rel_perm;
  MaterialProperty<Real> &_dens0;
  MaterialProperty<RealVectorValue> &_gravity;
  MaterialProperty<RealTensorValue> & _permeability;

  MaterialProperty<Real> &_density_n;
  MaterialProperty<Real> &_rel_perm_n;
  MaterialProperty<Real> &_dens0_n;
  MaterialProperty<RealVectorValue> &_gravity_n;
  MaterialProperty<RealTensorValue> & _permeability_n;
};

#endif /* RICHARDSFLUXJUMPINDICATOR_H */
