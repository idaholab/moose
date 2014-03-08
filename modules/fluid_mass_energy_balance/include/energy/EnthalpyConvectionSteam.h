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

#ifndef ENTHALPYCONVECTIONSTEAM
#define ENTHALPYCONVECTIONSTEAM

#include "Kernel.h"
#include "Material.h"

//Forward Declarations
class EnthalpyConvectionSteam;

template<>
InputParameters validParams<EnthalpyConvectionSteam>();

class EnthalpyConvectionSteam : public Kernel
{
public:

  EnthalpyConvectionSteam(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

    MaterialProperty<Real> & _Dtau_steamDH;
    MaterialProperty<Real> & _Dtau_steamDP;
    MaterialProperty<RealGradient> & _darcy_mass_flux_steam;
    MaterialProperty<Real> & _tau_steam;
    MaterialProperty<Real> &_enthalpy_steam;
    MaterialProperty<Real> &_denthalpy_steamdH_P;
    MaterialProperty<Real> &_denthalpy_steamdP_H;

    unsigned int  _p_var;
    VariableGradient & _grad_p;

};
#endif //ENTHALPYCONVECTIONSTEAM
