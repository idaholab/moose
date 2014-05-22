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

#ifndef WATERMASSFLUXELEVATIONPT_H
#define WATERMASSFLUXELEVATIONPT_H

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class WaterMassFluxElevationPT;

template<>
InputParameters validParams<WaterMassFluxElevationPT>();

class WaterMassFluxElevationPT : public Kernel
{
public:

  WaterMassFluxElevationPT(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

    MaterialProperty<Real> & _density_water;
    MaterialProperty<Real> & _tau_water;
    MaterialProperty<Real> & _gravity;
    MaterialProperty<RealVectorValue> & _gravity_vector;

};
#endif //WATERMASSFLUXELEVATION_H
