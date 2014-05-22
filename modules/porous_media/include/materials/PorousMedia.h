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

#ifndef POROUSMEDIA_H
#define POROUSMEDIA_H

#include "Material.h"


//Forward Declarations
class PorousMedia;

template<>
InputParameters validParams<PorousMedia>();

/**
 * Simple material with PorousMedia properties.
 */
class PorousMedia : public Material
{
public:
  PorousMedia(const std::string & name,
              InputParameters parameters);

protected:
  //virtual void initQpStatefulProperties();
  virtual void computeProperties();
////Grab user input parameters
  //rock property inputs
  Real _input_permeability;
  Real _input_porosity;
  Real _input_compressibility;
  Real _input_density_rock;
  //gravity inputs
  Real _input_gravity;
  Real _gx;
  Real _gy;
  Real _gz;

  bool _has_chem_reactions;

////Declare material properties
  //rock material props
  MaterialProperty<Real> & _permeability;
  MaterialProperty<Real> & _porosity;
  MaterialProperty<Real> & _compressibility;
  MaterialProperty<Real> * _porosity_old;
  MaterialProperty<Real> & _density_rock;
  //gravity material props
  MaterialProperty<Real> & _gravity;
  MaterialProperty<RealVectorValue> & _gravity_vector;

    void setPropsComputed(bool value) { _already_computed = value; }

    bool areParentPropsComputed() const { return _already_computed; }

private:

    /**
     * This parameter is here to indicate whether or not the PorousMedia Material
     * properties have been computed.  Each of the classes in the diamond hierarchy
     * should query this variable (using the accessor) to determine whether or
     * not they should recompute the base class properties
     */
    bool _already_computed;

};


#endif //POROUSMEDIA_H
