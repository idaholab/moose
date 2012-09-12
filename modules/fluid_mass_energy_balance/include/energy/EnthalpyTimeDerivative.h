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

#include "TimeDerivative.h"
#include "WaterSteamEOS.h"

#ifndef ENTHALPYTIMEDERIVATIVE
#define ENTHALPYTIMEDERIVATIVE

//Forward Declarations
class EnthalpyTimeDerivative;
class WaterSteamEOS;

template<>
InputParameters validParams<EnthalpyTimeDerivative>();

class EnthalpyTimeDerivative : public TimeDerivative
{
public:
    
    EnthalpyTimeDerivative(const std::string & name, InputParameters parameters);
    
protected:
    virtual Real computeQpResidual();
    virtual Real computeQpJacobian();
    virtual Real computeQpOffDiagJacobian(unsigned int jvar);
    
    const WaterSteamEOS & _water_steam_properties;  
    
    MaterialProperty<Real> & _density;               //changed from VariableValue
    MaterialProperty<Real> & _time_old_density;      //changed from VariableValue
    
    //VariableValue & _temperature;
    //VariableValue  & _temperature_old;
    MaterialProperty<Real> & _temperature;
    MaterialProperty<Real> & _time_old_temperature;       //added by kat
    
    MaterialProperty<Real> & _dTdH_P;                //changed from VariableValue
    MaterialProperty<Real> & _dTdP_H;                //changed from VariableValue
    MaterialProperty<Real> & _ddensitydH_P;          //changed from VariableValue
    MaterialProperty<Real> & _ddensitydp_H;          //changed from VariableValue
    
    VariableValue  & _pressure_old;                 
    unsigned int _p_var;
  
    //  VariableValue  & _porosity_old;
    MaterialProperty<Real> & _porosity;
    MaterialProperty<Real> & _specific_heat_rock;
    MaterialProperty<Real> & _density_rock;
    
    VariableValue & _u_old;
};
#endif //ENTHALPYTIMEDERIVATIVE
