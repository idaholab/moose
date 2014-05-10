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

#ifndef NODALVALUES_H
#define NODALVALUES_H

#include "NodalVariableVectorPostprocessor.h"

//Forward Declarations
class NodalValueSampler;

template<>
InputParameters validParams<NodalValueSampler>();

class NodalValueSampler : public NodalVariableVectorPostprocessor
{
public:
  NodalValueSampler(const std::string & name, InputParameters parameters);

  virtual ~NodalValueSampler() {}

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  virtual void threadJoin(const UserObject & y);

protected:
  unsigned int _sort_by;

  /// x coordinate of the points
  VectorPostprocessorValue & _x;
  /// y coordinate of the points
  VectorPostprocessorValue & _y;
  /// x coordinate of the points
  VectorPostprocessorValue & _z;

  /// The node ID of each point
  VectorPostprocessorValue & _id;

  std::vector<VectorPostprocessorValue *> _values;

  /// x coordinate of the points
  VectorPostprocessorValue _x_tmp;
  /// y coordinate of the points
  VectorPostprocessorValue _y_tmp;
  /// x coordinate of the points
  VectorPostprocessorValue _z_tmp;

  /// The node ID of each point
  VectorPostprocessorValue _id_tmp;

  std::vector<VectorPostprocessorValue> _values_tmp;
};

#endif
