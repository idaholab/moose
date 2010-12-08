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

#ifndef DGMATDIFFUSION_H
#define DGMATDIFFUSION_H

#include "DGKernel.h"

//Forward Declarations
class DGMatDiffusion;

template<>
InputParameters validParams<DGMatDiffusion>();

/**
 * DG kernel for diffusion with material property
 *
 * General DG kernel that this class can handle is:
 * { \grad u * n_e} [v] + epsilon { \grad v * n_e } [u] + (sigma / |e| * [u][v])
 *
 *  [a] = [ a_1 - a_2 ]
 *  {a} = 0.5 * (a_1 + a_2)
 *
 */
class DGMatDiffusion : public DGKernel
{
public:
  DGMatDiffusion(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual(DGResidualType type);
  virtual Real computeQpJacobian(DGJacobianType type);

  Real _epsilon;
  Real _sigma;

  std::string _prop_name;                       // name of the material property
  MaterialProperty<Real> & _diff;               // diffusivity
  MaterialProperty<Real> & _diff_neighbor;      // diffusivity
};
 
#endif
