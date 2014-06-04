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

#ifndef CHEMICALREACTIONS_H
#define CHEMICALREACTIONS_H

#include "PorousMedia.h"


//Forward Declarations
class ChemicalReactions;

template<>
InputParameters validParams<ChemicalReactions>();

/**
 * Simple material with PorousMedia properties.
 */
class ChemicalReactions : virtual public PorousMedia
{
public:
  ChemicalReactions(const std::string & name,
              InputParameters parameters);

protected:
  virtual void computeProperties();
////Grab user input parameters
  Real _input_chem_diff;
  std::vector<Real> _mineral;
  std::vector<Real> _molecular_weight;
  std::vector<Real> _mineral_density;
  std::vector<VariableValue *> _vals;

////Get coupled permeability aux varaible
  Real _input_permeability;
  Real _input_porosity;

////Declare material properties
  MaterialProperty<Real> & _diffusivity;

};


#endif //POROUSMEDIA_H
