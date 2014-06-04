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

#ifndef TEMPERATUREDIFFUSION
#define TEMPERATUREDIFFUSION

#include "Diffusion.h"
#include "Material.h"

class TemperatureDiffusion;

template<>
InputParameters validParams<TemperatureDiffusion>();

class TemperatureDiffusion : public Diffusion
{
public:

  TemperatureDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  MaterialProperty<Real> &_thermal_conductivity;

};
#endif //TEMPERATUREDIFFUSION
