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
#ifndef EXAMPLESHAPEELEMENTKERNEL_H
#define EXAMPLESHAPEELEMENTKERNEL_H

#include "Kernel.h"
#include "ExampleShapeElementUserObject.h"

class ExampleShapeElementKernel : public Kernel
{
public:
  ExampleShapeElementKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const ExampleShapeElementUserObject & _shp;
  const Real & _shp_integral;
  const std::vector<Real> & _shp_jacobian;

  unsigned int _u_var;
  const std::vector<dof_id_type> & _u_dofs;
  unsigned int _v_var;
  const std::vector<dof_id_type> & _v_dofs;
};

template<>
InputParameters validParams<ExampleShapeElementKernel>();

#endif //EXAMPLESHAPEELEMENTKERNEL_H
