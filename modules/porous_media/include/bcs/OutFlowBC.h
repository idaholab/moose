/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef OUTFLOWBC_H
#define OUTFLOWBC_H

#include "IntegratedBC.h"
#include "Material.h"
//libMesh includes
#include "libmesh/vector_value.h"


//Forward Declarations
class OutFlowBC;

template<>
InputParameters validParams<OutFlowBC>();

/**
 * Implements a simple constant VectorNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class OutFlowBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  OutFlowBC(const std::string & name, InputParameters parameters);

 virtual ~OutFlowBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /**
   * Vector to dot with the normal.
   */
  MaterialProperty<Real> &_thermal_conductivity;
  MaterialProperty<Real> & _specific_heat_water;
  MaterialProperty<RealGradient> & _darcy_mass_flux_water;

//  std::vector<RealGradient> & _grad_p;

};

#endif //NEUMANNBC_H
