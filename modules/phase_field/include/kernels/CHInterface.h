/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHINTERFACE_H
#define CHINTERFACE_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class CHInterface;

template<>
InputParameters validParams<CHInterface>();

/**
 * This is the Cahn-Hilliard equation base class that implements the interfacial or gradient energy term of the equation.
 * See M.R. Tonks et al. / Computational Materials Science 51 (2012) 20–29 for more information.
 */
class CHInterface : public Kernel
{
public:
  CHInterface(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  std::string _kappa_name;
  std::string _mob_name;
  std::string _Dmob_name;
  std::string _grad_mob_name;

  const MaterialProperty<Real> & _kappa;
  const MaterialProperty<Real> & _M;
  bool _has_MJac;
  const MaterialProperty<Real> * _DM;
  const MaterialProperty<RealGradient> & _grad_M;
  const MaterialProperty<RealGradient> * _Dgrad_Mnp;
  const MaterialProperty<Real> * _Dgrad_Mngp;

  VariableSecond & _second_u;
  VariableTestSecond & _second_test;
  VariablePhiSecond & _second_phi;
};

#endif //CHINTERFACE_H
