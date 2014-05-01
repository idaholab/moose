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

#ifndef MATERIALREALTENSORVALUEAUX_H
#define MATERIALREALTENSORVALUEAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declerations
class MaterialRealTensorValueAux;

template<>
InputParameters validParams<MaterialRealTensorValueAux>();

/**
 * AuxKernel for outputting a RealTensorValue material property component to an AuxVariable
 */
class MaterialRealTensorValueAux : public MaterialAuxBase<RealTensorValue>
{
public:

  /**
   * Class constructor
   * @param name The name of the AuxKernel
   * @param parameters The input parameters for this AuxKernel
   */
  MaterialRealTensorValueAux(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MaterialRealTensorValueAux();

protected:

  /**
   * Computes the component of the tensor for output
   */
  virtual Real computeValue();

  /// The row index to output
  unsigned int _row;

  /// The column index to output
  unsigned int _col;
};

#endif //MATERIALREALTENSORVALUEAUX_H
