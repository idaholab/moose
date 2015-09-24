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

#include "MaterialStdVectorAuxBase.h"

// Forward declarations
class MaterialStdVectorAux;

template<>
InputParameters validParams<MaterialStdVectorAux>();

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
class MaterialStdVectorAux : public MaterialStdVectorAuxBase<Real>
{
public:

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialStdVectorAux(const InputParameters & parameters);

  virtual ~MaterialStdVectorAux();

protected:
  /// Returns the value of the material property for the given index
  virtual Real getRealValue();
};

#endif //MATERIALSTDVECTORAUX_H
