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


#include "ImplicitEuler.h"

#ifndef ENTHALPYIMPLICITEULER
#define ENTHALPYIMPLICITEULER

//Forward Declarations
class EnthalpyImplicitEuler;

template<>
InputParameters validParams<EnthalpyImplicitEuler>();

class EnthalpyImplicitEuler : public ImplicitEuler
{
public:

  EnthalpyImplicitEuler(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

//  VariableValue & _temperature;
//  VariableValue & _temperature_old;
//  VariableValue & _density;
//  VariableValue & _density_old;
  
  MaterialProperty<Real> & _temperature;
  MaterialProperty<Real> & _temperature_old;
  MaterialProperty<Real> & _density;
  MaterialProperty<Real> & _density_old;

  MaterialProperty<Real> & _porosity;
  MaterialProperty<Real> & _density_rock;

  VariableValue & _u_old;
};
#endif //ENTHALPYIMPLICITEULER
