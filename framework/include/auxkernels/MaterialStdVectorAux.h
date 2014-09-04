/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALSTDVECTORAUX_H
#define MATERIALSTDVECTORAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declarations
class MaterialStdVectorAux;

template<>
InputParameters validParams<MaterialStdVectorAux>();

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
class MaterialStdVectorAux : public MaterialAuxBase<std::vector<Real> >
{
public:

  /**
   * Class constructor
   * @param name AuxKernel name
   * @param parameters The input parameters for this object
   */
  MaterialStdVectorAux(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MaterialStdVectorAux();

protected:

  /**
   * Compute the value of the material property for the given index
   */
  virtual Real computeValue();

  /// The vector index to output
  unsigned int _index;
};

#endif //MATERIALSTDVECTORAUX_H
