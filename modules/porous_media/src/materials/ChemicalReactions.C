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

#include "ChemicalReactions.h"

template<>
InputParameters validParams<ChemicalReactions>()
{
  InputParameters params = validParams<PorousMedia>();
  //chemical reaction coupled variables and paramerters
  params.addParam<Real>("diffusivity", 1e-8, "the chemical diffusicity, [m^2/s]");
  params.addParam<std::vector<Real> >("mineral", std::vector<Real>(1, 16.65), "Initial mineral concentration, [mol/L] solution");
  params.addParam<std::vector<Real> >("molecular_weight", std::vector<Real>(1, 100.08), "The molecular weight of mineral, [g/mol]");
  params.addParam<std::vector<Real> >("mineral_density", std::vector<Real>(1, 2.5), "The density of mineral [g/cm^3]");
  params.addCoupledVar("v", "caco3");
  
  return params;
}

ChemicalReactions::ChemicalReactions(const std::string & name,
                         InputParameters parameters)
  :PorousMedia(name, parameters),
////Grab user input parameters
   _input_chem_diff(getParam<Real>("diffusivity")),
   _mineral(getParam<std::vector<Real> >("mineral")),
   _molecular_weight(getParam<std::vector<Real> >("molecular_weight")),
   _mineral_density(getParam<std::vector<Real> >("mineral_density")),

   _input_permeability(getParam<Real>("permeability")),
   _input_porosity(getParam<Real>("porosity")),

////Delcare material properties
   _diffusivity(declareProperty<Real>("diffusivity"))

{
    //resize and fill in _vals with number of provided chem species
    int n = coupledComponents("v");
    _vals.resize(n);
    for (unsigned int i=0; i<_vals.size(); ++i)
        _vals[i] = &coupledValue("v", i);
    
}

void
ChemicalReactions::computeProperties()
{
    if (!areParentPropsComputed())
        PorousMedia::computeProperties();
    
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
      //chem reactions
      _diffusivity[qp]     = _input_chem_diff;
      
      // if dissolution or precipitation is taking place, we need to adjust permeability and porosity accordingly
      if (_vals.size())
      {
          Real _initial_vf = 1.0;
          Real _vf = 1.0;
              
          for (unsigned int i=0; i<_vals.size(); ++i)
          {
              _initial_vf += 1.0e-3*_mineral[i]*_molecular_weight[i]/_mineral_density[i];
              
              _vf += 1.0e-3*(*_vals[i])[qp]*_molecular_weight[i]/_mineral_density[i];
                  
          }
          // Update porosity
          _porosity[qp] = _initial_vf *_input_porosity/_vf;
          
      }
          
      if (_porosity[qp] < 1.0e-3)
          _porosity[qp]=1.0e-3;
          
      // Permeability changes calculated from porosity changes according to Carman-Kozeny relationship k=ki*(1-ni)^2 * (n/ni)^3 / (1-n)^2
      _permeability[qp] = _input_permeability * (1.0-_input_porosity) * (1.0-_input_porosity) * std::pow(_porosity[qp]/_input_porosity,3)/(1.0-_porosity[qp])/(1.0-_porosity[qp]);
          
      // Permeability changes calculated from porosity changes according to Power Law with an order of 5.2: k=ki* (n/ni)^5.2
      //_permeability[qp] = input_permeability * std::pow(_porosity[qp]/_input_porosity, 5.2);
          
      // The diffusivity used in the kernels (already multiplied by porosity)
      _diffusivity[qp] = _input_chem_diff*_porosity[qp];
  }  
}
