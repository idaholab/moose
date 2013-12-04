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

#ifndef RICHARDSBOREHOLE_H
#define RICHARDSBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class RichardsBorehole;

template<>
InputParameters validParams<RichardsBorehole>();

class RichardsBorehole : public DiracKernel
{
public:
  RichardsBorehole(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  Real _well_constant_production;
  Real _well_constant_injection;
  Real _p_bot;
  RealVectorValue _gravity;

  MaterialProperty<Real> &_dens0;
  MaterialProperty<Real> &_viscosity;

  MaterialProperty<Real> &_dseff;

  MaterialProperty<Real> &_rel_perm;
  MaterialProperty<Real> &_drel_perm;

  MaterialProperty<Real> &_density;
  MaterialProperty<Real> &_ddensity;

  PostprocessorValue & _reporter;
  std::string _point_file;

  MaterialProperty<RealVectorValue> &_vel_SUPG;
  MaterialProperty<RealTensorValue> &_vel_prime_SUPG;
  MaterialProperty<Real> &_tau_SUPG;
  MaterialProperty<RealVectorValue> &_tau_prime_SUPG;

  std::vector<Real> _xs;
  std::vector<Real> _ys;
  std::vector<Real> _zs;
  Point _bottom_point;

  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);
};

#endif //RICHARDSBOREHOLE_H
