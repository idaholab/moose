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

#ifndef VACUUMBC_H
#define VACUUMBC_H

#include "IntegratedBC.h"


//Forward Declarations
class VacuumBC;

template<>
InputParameters validParams<VacuumBC>();

/**
 * Implements a simple Vacuum BC for neutron diffusion on the boundary.
 * Vacuum BC is defined as \f$ D\frac{du}{dn}+\frac{u}{2} = 0\f$, where u is neutron flux.
 * Hence, \f$ D\frac{du}{dn}=-\frac{u}{2} \f$ and \f$ -\frac{u}{2} \f$ is substituted into 
 * the Neumann BC term produced from integrating the diffusion operator by parts.
 */
class VacuumBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VacuumBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  Real _alpha;                                  /// Ratio of u to du/dn
};

#endif //VACUUMBC_H
