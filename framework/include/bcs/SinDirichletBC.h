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

#ifndef SINDIRICHLETBC_H
#define SINDIRICHLETBC_H

#include "libmesh/libmesh_common.h"
#include "NodalBC.h"


//Forward Declarations
class SinDirichletBC;

template<>
InputParameters validParams<SinDirichletBC>();

class SinDirichletBC : public NodalBC
{
public:

  SinDirichletBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

private:
  Real _initial;
  Real _final;
  Real _duration;
};

#endif
