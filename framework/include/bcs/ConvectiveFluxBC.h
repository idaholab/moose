/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONVECTIVEFLUXBC_H
#define CONVECTIVEFLUXBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class ConvectiveFluxBC;

template<>
InputParameters validParams<ConvectiveFluxBC>();

class ConvectiveFluxBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConvectiveFluxBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~ConvectiveFluxBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  
private:
  /**
   * Ratio of u to du/dn
   */
  Real _initial;
  Real _final;
  Real _rate;
  Real _duration;
};

#endif //CONVECTIVEFLUXBC_H
