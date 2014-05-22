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

#ifndef WATERMASSFLUXPRESSUREPT_H
#define WATERMASSFLUXPRESSUREPT_H

#include "Diffusion.h"
#include "Material.h"

//Forward Declarations
class WaterMassFluxPressurePT;

template<>
InputParameters validParams<WaterMassFluxPressurePT>();

class WaterMassFluxPressurePT : public Diffusion
{
public:

  WaterMassFluxPressurePT(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MaterialProperty<Real> & _tau_water;
};
#endif //WATERMASSFLUXPRESSUREPT_H
