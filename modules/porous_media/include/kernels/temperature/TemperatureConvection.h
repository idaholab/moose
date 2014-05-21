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

#ifndef TEMPERATURECONVECTION
#define TEMPERATURECONVECTION

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class TemperatureConvection;

template<>
InputParameters validParams<TemperatureConvection>();

class TemperatureConvection : public Kernel
{
public:

  TemperatureConvection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();  

  MaterialProperty<Real> & _specific_heat_water;
  MaterialProperty<RealGradient> & _darcy_mass_flux_water;
//  MooseArray<RealGradient> &_pore_velocity_water;
  
};
#endif //TEMPERATURECONVECTION
