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

#ifndef VALUETHRESHOLDMARKER_H
#define VALUETHRESHOLDMARKER_H

#include "Marker.h"

// libmesh includes
#include "mesh_tools.h"

class ValueThresholdMarker;

template<>
InputParameters validParams<ValueThresholdMarker>();

class ValueThresholdMarker : public Marker
{
public:
  ValueThresholdMarker(const std::string & name, InputParameters parameters);
  virtual ~ValueThresholdMarker(){};

protected:
  virtual int computeElementMarker();

  bool _coarsen_set;
  Real _coarsen;
  bool _refine_set;
  Real _refine;

  bool _invert;
  bool _dont_mark;

  VariableName _variable_name;
  MooseVariable & _variable;
  SystemBase & _variable_sys;
  const NumericVector<Number> * & _variable_sys_solution;
  unsigned int _variable_number;
  const DofMap & _variable_dof_map;
  FEType _variable_fe_type;
  std::vector<unsigned int> _variable_dof_indices;
};

#endif /* VALUETHRESHOLDMARKER_H */
