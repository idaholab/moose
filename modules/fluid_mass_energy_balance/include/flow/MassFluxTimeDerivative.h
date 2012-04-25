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

#ifndef MASSFLUXTIMEDERIVATIVE
#define MASSFLUXTIMEDERIVATIVE

//Forward Declarations
class MassFluxTimeDerivative;

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
  
  VariableValue  & _density;
  VariableValue & _density_old;
  VariableValue  & _ddensitydp_H; //derivative of water density vs. pressure
  VariableValue & _enthalpy_old;
  unsigned int _h_var;
  VariableValue & _ddensitydH_P;
  
    
  MaterialProperty<Real> & _porosity;  
//  VariableValue  & _porosity;
//  VariableValue  & _porosity_old;

  VariableValue & _u_old;
};
#endif //MASSFLUXTIMEDERIVATIVE
