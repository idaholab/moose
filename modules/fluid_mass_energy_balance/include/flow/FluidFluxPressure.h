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

#ifndef FLUIDFLUXPRESSURE
#define FLUIDFLUXPRESSURE

#include "Diffusion.h"
#include "Material.h"

//Forward Declarations
class FluidFluxPressure;

template<>
InputParameters validParams<FluidFluxPressure>();

class FluidFluxPressure : public Diffusion
{
public:

    FluidFluxPressure(const std::string & name, InputParameters parameters);

protected:
    virtual Real computeQpResidual();

    virtual Real computeQpJacobian();

    MaterialProperty<Real> & _tau_water;
    MaterialProperty<RealGradient> & _darcy_mass_flux_water;
};
#endif //FLUIDFLUXPRESSURE
