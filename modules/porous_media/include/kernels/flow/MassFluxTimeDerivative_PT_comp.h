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

#ifndef MASSFLUXTIMEDERIVATIVE_PT_COMP
#define MASSFLUXTIMEDERIVATIVE_PT_COMP

//Forward Declarations
class MassFluxTimeDerivative_PT_comp;

template<>
InputParameters validParams<MassFluxTimeDerivative_PT_comp>();

class MassFluxTimeDerivative_PT_comp : public TimeDerivative
{
public:

  MassFluxTimeDerivative_PT_comp(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

    MaterialProperty<Real> & _density_water;
    MaterialProperty<Real> & _density_water_old;
    MaterialProperty<Real> & _dwdp;
    
    bool _has_chem_reactions;
    MaterialProperty<Real> & _porosity;
    MaterialProperty<Real> & _compressibility;
    MaterialProperty<Real> * _porosity_old;
  VariableValue & _u_old;  

};
#endif //MASSFLUXTIMEDERIVATIVE_PT_COMP
