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

#ifndef ENTHALPYTIMEDERIVATIVE
#define ENTHALPYTIMEDERIVATIVE

//Forward Declarations
class EnthalpyTimeDerivative;

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
  
    
    VariableValue  & _density;
    VariableValue  & _density_old;
    VariableValue  & _temperature;
    VariableValue  & _temperature_old;
    VariableValue  & _dTdH_P; //derivative of water density vs. temperature
    VariableValue  & _dTdP_H;
    VariableValue  & _ddensitydH_P;
    VariableValue  & _pressure_old;
    unsigned int _p_var;
    VariableValue & _ddensitydp_H;
  
    //  VariableValue  & _porosity_old;
    MaterialProperty<Real> & _porosity;
    MaterialProperty<Real> & _specific_heat_rock;
    MaterialProperty<Real> & _density_rock;
    
  VariableValue & _u_old;
};
#endif //ENTHALPYTIMEDERIVATIVE
