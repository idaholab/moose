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

#ifndef DGMDDBC_H
#define DGMDDBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class DGMDDBC;
class Function;

template<>
InputParameters validParams<DGMDDBC>();

/**
 * Implements a simple dirichlet BC for DG with material property
 *
 * BC derived from diffusion problem that can handle:
 * { \grad u * n_e} [v] + epsilon { \grad v * n_e } [u] + (sigma / |e| * [u][v])
 *
 *  [a] = [ a_1 - a_2 ]
 *  {a} = 0.5 * (a_1 + a_2)
 */
class DGMDDBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DGMDDBC(const std::string & name, InputParameters parameters);
    
  virtual ~DGMDDBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Function & _func;

  std::string _prop_name;                       // name of the material property
  MaterialProperty<Real> & _diff;               // diffusivity

  Real _epsilon;
  Real _sigma;
};

#endif //DGMDDBC_H
