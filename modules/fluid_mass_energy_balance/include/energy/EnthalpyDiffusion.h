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

#ifndef ENTHALPYDIFFUSION
#define ENTHALPYDIFFUSION

#include "Diffusion.h"
#include "Material.h"

class EnthalpyDiffusion;

template<>
InputParameters validParams<EnthalpyDiffusion>();

class EnthalpyDiffusion : public Diffusion
{
public:

  EnthalpyDiffusion(const std::string & name, InputParameters parameters);
    
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  //std::string _prop_name_dTdH_P;       //(added by Kat)
  //std::string _prop_name_dTdP_H;       //(added by Kat)
    VariableGradient & _grad_T;
    //VariableValue & _dTdH_P;      //(removed by Kat)
    //VariableValue & _dTdP_H;      //(removed by Kat)
    unsigned int _p_var;
    MaterialProperty<Real> &_thermal_conductivity;
      MaterialProperty<Real> &_dTdP_H;    //(added by Kat)
    MaterialProperty<Real> &_dTdH_P;    //(added by Kat)
  
};
#endif //ENTHALPYDIFFUSION
