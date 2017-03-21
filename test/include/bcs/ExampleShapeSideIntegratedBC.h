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

#ifndef EXAMPLESHAPESIDEINTEGRATEDBC_H
#define EXAMPLESHAPESIDEINTEGRATEDBC_H

#include "NonlocalIntegratedBC.h"
#include "NumShapeSideUserObject.h"
#include "DenomShapeSideUserObject.h"

class ExampleShapeSideIntegratedBC : public NonlocalIntegratedBC
{
public:
  ExampleShapeSideIntegratedBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  /// new method for on-diagonal jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalJacobian(dof_id_type dof_index);
  /// new method for off-diagonal jacobian contributions corresponding to non-local dofs
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index);

  const NumShapeSideUserObject & _num_shp;
  const Real & _num_shp_integral;
  const std::vector<Real> & _num_shp_jacobian;
  const DenomShapeSideUserObject & _denom_shp;
  const Real & _denom_shp_integral;
  const std::vector<Real> & _denom_shp_jacobian;

  const std::vector<dof_id_type> & _var_dofs;
  unsigned int _v_var;
  const std::vector<dof_id_type> & _v_dofs;
  Real _Vb;
};

template <>
InputParameters validParams<ExampleShapeSideIntegratedBC>();

#endif // EXAMPLESHAPESIDEINTEGRATEDBC_H
