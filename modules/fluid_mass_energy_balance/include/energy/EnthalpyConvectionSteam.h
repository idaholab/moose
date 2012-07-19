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
    std::string _prop_name_enthalpy_steam;
    std::string _prop_name_denthalpy_steamdH_P;
    std::string _prop_name_denthalpy_steamdP_H;
    MaterialProperty<Real> &_enthalpy_steam;            //(added by Kat)
    MaterialProperty<Real> &_denthalpy_steamdH_P;       //(added by Kat)
    MaterialProperty<Real> &_denthalpy_steamdP_H;       //(added by Kat)
    //VariableGradient & _grad_enthalpy_steam;
    //VariableValue & _enthalpy_steam;                  //(removed by Kat)
    //VariableValue & _denthalpy_steamdH_P;             //(removed by Kat)
    //VariableValue & _denthalpy_steamdP_H;             //(removed by Kat)
    unsigned int  _p_var;
    VariableGradient & _grad_p;
  
};
#endif //ENTHALPYCONVECTIONSTEAM
