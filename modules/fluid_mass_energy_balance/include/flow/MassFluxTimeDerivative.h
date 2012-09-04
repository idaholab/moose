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

#ifndef MASSFLUXTIMEDERIVATIVE
#define MASSFLUXTIMEDERIVATIVE

//Forward Declarations
class MassFluxTimeDerivative;
class WaterSteamEOS;

template<>
InputParameters validParams<MassFluxTimeDerivative>();

class MassFluxTimeDerivative : public TimeDerivative
{
public:

  MassFluxTimeDerivative(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
    
    const WaterSteamEOS & _water_steam_properties;
  
    MaterialProperty<Real> & _density;             //changed from VariableValue
    MaterialProperty<Real> & _time_old_density;  //changed from VariableValue
    MaterialProperty<Real> & _ddensitydp_H;        //changed from VariableValue
    MaterialProperty<Real> & _ddensitydH_P;        //changed from VariableValue

    VariableValue & _enthalpy_old;
    unsigned int _h_var;
  
    
    MaterialProperty<Real> & _porosity;  
//  VariableValue  & _porosity;
//  VariableValue  & _porosity_old;

    VariableValue & _u_old;
};
#endif //MASSFLUXTIMEDERIVATIVE
