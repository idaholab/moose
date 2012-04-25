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

#ifndef WATERMASSFLUXELEVATION
#define WATERMASSFLUXELEVATION

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class WaterMassFluxElevation;

template<>
InputParameters validParams<WaterMassFluxElevation>();

class WaterMassFluxElevation : public Kernel
{
public:

  WaterMassFluxElevation(const std::string & name, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();


  VariableValue  & _density_water;

  
  MaterialProperty<Real> & _tau_water;
  MaterialProperty<Real> & _gravity;
  MaterialProperty<RealVectorValue> & _gravity_vector;

};
#endif //WATERMASSFLUXELEVATION
