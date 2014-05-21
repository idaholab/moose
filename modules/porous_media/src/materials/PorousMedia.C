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

#include "PorousMedia.h"
#include "Transient.h"

template<>
InputParameters validParams<PorousMedia>()
{
  InputParameters params = validParams<Material>();
  //rock property inputs
  params.addParam<Real>("permeability",1.0e-12, "intrinsic permeability in [m^2]");
  params.addParam<Real>("porosity", 0.3, "rock porosity");
  params.addParam<Real>("compressibility", 1.0e-5, "total compressibility of the researvoir");  
  params.addParam<Real>("density_rock", 2.50e3, "rock density, [kg/m^3]");
  //gravity inputs
  params.addParam<Real>("gravity",9.80665,"gravity acceleration constant, [m/s^2]");
  params.addParam<Real>("gx",0.0,"x component of the gravity pressure vector");
  params.addParam<Real>("gy",0.0,"y component of the gravity pressure vector");
  params.addParam<Real>("gz",1.0,"z component of the gravity pressure vector");
    
  //flag if chemical reactions are present.  determines whether porosity_old is called
  params.addParam<bool>("has_chem_reactions", false, "add discription");
    
  return params;
}

PorousMedia::PorousMedia(const std::string & name,
                         InputParameters parameters)
  :Material(name, parameters),
////Grab user input parameters
   //rock property inputs
   _input_permeability(getParam<Real>("permeability")),
   _input_porosity(getParam<Real>("porosity")),
   _input_compressibility(getParam<Real>("compressibility")),
   _input_density_rock(getParam<Real>("density_rock")),
   //gravity inputs
   _input_gravity(getParam<Real>("gravity")),
   _gx(getParam<Real>("gx")),
   _gy(getParam<Real>("gy")),
   _gz(getParam<Real>("gz")),
   //chemical reactions
   _has_chem_reactions(getParam<bool>("has_chem_reactions")),

////Delcare material properties
   //rock material props
   _permeability(declareProperty<Real>("permeability")),
   _porosity(declareProperty<Real>("porosity")),
   _compressibility(declareProperty<Real>("compressibility")),
   //do we have chemical reactions happening? then we need to declare porosity_old
   _porosity_old(_has_chem_reactions ? &declarePropertyOld<Real>("porosity") : NULL),
   _density_rock(declareProperty<Real>("density_rock")),
   //gravity material props
   _gravity(declareProperty<Real>("gravity")),
   _gravity_vector(declareProperty<RealVectorValue>("gravity_vector")),

    _already_computed(false)

{ }

/*void
StochasticPorousMedia::initQpStatefulProperties()
{
    _porosity[_qp] = _input_porosity;
}*/

void
PorousMedia::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    //porous media
    _permeability[qp]       = _input_permeability;
    _porosity[qp]           = _input_porosity;
    _compressibility[qp]    = _input_compressibility;
    _density_rock[qp]       = _input_density_rock;
      
    //gravity    
    _gravity_vector[qp](0) = _gx; 
    _gravity_vector[qp](1) = _gy;
    _gravity_vector[qp](2) = _gz;
    _gravity[qp]           = _input_gravity;
  }
}
