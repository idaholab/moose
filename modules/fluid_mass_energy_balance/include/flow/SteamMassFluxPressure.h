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

#ifndef STEAMMASSFLUXPRESSURE
#define STEAMMASSFLUXPRESSURE

#include "Diffusion.h"
#include "Material.h"

//Forward Declarations
class SteamMassFluxPressure;

template<>
InputParameters validParams<SteamMassFluxPressure>();

class SteamMassFluxPressure : public Diffusion
{
public:

  SteamMassFluxPressure(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _h_var;
  VariableGradient & _grad_p;
  MaterialProperty<Real> & _Dtau_steamDP;
  MaterialProperty<Real> & _Dtau_steamDH;
  MaterialProperty<Real> &_tau_steam;
  MaterialProperty<RealGradient> & _darcy_mass_flux_steam;
};
#endif //STEAMMASSFLUXPRESSURE
