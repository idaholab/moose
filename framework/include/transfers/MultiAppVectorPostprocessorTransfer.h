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

#ifndef MULTIAPPVECTORPOSTPROCESSORTRANSFER_H
#define MULTIAPPVECTORPOSTPROCESSORTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppVectorPostprocessorTransfer;

template <>
InputParameters validParams<MultiAppVectorPostprocessorTransfer>();

/**
 * Copies the values of a VectorPostprocessor from the Master to postprocessors
 * on each MultiApp
 * or collects the postprocessors on each MultiApp into a VectorPostprocessor
 */
class MultiAppVectorPostprocessorTransfer : public MultiAppTransfer
{
public:
  MultiAppVectorPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  virtual void executeToMultiapp();
  virtual void executeFromMultiapp();

  const PostprocessorName & _sub_pp_name;
  const VectorPostprocessorName & _master_vpp_name;
  const std::string & _vector_name;
};

#endif /* MultiAppVectorPostprocessorTransfer_H */
