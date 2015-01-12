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

#ifndef MATERIALREALDENSEMATRIXAUX_H
#define MATERIALREALDENSEMATRIXAUX_H

// MOOSE includes
#include "MaterialAuxBase.h"

// Forward declarations
class MaterialRealDenseMatrixAux;

template<>
InputParameters validParams<MaterialRealDenseMatrixAux>();

/**
 * AuxKernel for outputting a DenseMatrix<Real> material property component to an AuxVariable
 */
class MaterialRealDenseMatrixAux : public MaterialAuxBase<DenseMatrix<Real> >
{
public:

  /**
   * Class constructor
   * @param name The name of the AuxKernel
   * @param parameters The input parameters for this AuxKernel
   */
  MaterialRealDenseMatrixAux(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MaterialRealDenseMatrixAux();

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

#endif //MATERIALREALDENSEMATRIXAUX_H
