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

#ifndef GEOTHERMAL_H
#define GEOTHERMAL_H

#include "FluidFlow.h"
#include "HeatTransport.h"
//#include "SolidMechanics.h"
#include "ChemicalReactions.h"


//Forward Declarations
class Geothermal;

template<>
InputParameters validParams<Geothermal>();

/**
 * Simple material with Geothermal properties.
 */
//class Geothermal : public FluidFlow, public HeatTransport, public SolidMechanics, public ChemicalReactions
class Geothermal : public FluidFlow, public HeatTransport,  public ChemicalReactions
{
public:
  Geothermal(const std::string & name,
             InputParameters parameters);

protected:
  virtual void computeProperties();
};
#endif //GEOTHERMAL_H
