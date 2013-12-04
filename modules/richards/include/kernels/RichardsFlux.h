/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RICHARDSFLUX
#define RICHARDSFLUX

#include "Kernel.h"

// Forward Declarations
class RichardsFlux;

template<>
InputParameters validParams<RichardsFlux>();

class RichardsFlux : public Kernel
{
public:

  RichardsFlux(const std::string & name,
                        InputParameters parameters);


protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MaterialProperty<Real> &_dens0;
  MaterialProperty<Real> &_viscosity;
  MaterialProperty<RealVectorValue> &_gravity;
  MaterialProperty<RealTensorValue> & _permeability;

  MaterialProperty<Real> &_seff;
  MaterialProperty<Real> &_dseff;
  MaterialProperty<Real> &_d2seff;

  MaterialProperty<Real> &_rel_perm;
  MaterialProperty<Real> &_drel_perm;
  MaterialProperty<Real> &_d2rel_perm;

  MaterialProperty<Real> &_density;
  MaterialProperty<Real> &_ddensity;
  MaterialProperty<Real> &_d2density;

  VariableSecond & _second_u;
  VariablePhiSecond & _second_phi;

  MaterialProperty<RealVectorValue> &_vel_SUPG;
  MaterialProperty<RealTensorValue> &_vel_prime_SUPG;
  MaterialProperty<Real> &_tau_SUPG;
  MaterialProperty<RealVectorValue> &_tau_prime_SUPG;

};

#endif //RICHARDSFLUX
