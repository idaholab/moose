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

#ifdef LIBMESH_HAVE_DTK

#ifndef MULTIAPPDTKINTERPOLATIONTRANSFER_H
#define MULTIAPPDTKINTERPOLATIONTRANSFER_H

#include "MultiAppTransfer.h"
#include "MooseVariableInterface.h"
#include "DTKInterpolationHelper.h"

class MooseVariable;
class MultiAppDTKInterpolationTransfer;

template<>
InputParameters validParams<MultiAppDTKInterpolationTransfer>();

/**
 * Transfers from spatially varying Interpolations in a MultiApp to the "master" system.
 */
class MultiAppDTKInterpolationTransfer :
  public MultiAppTransfer
{
public:
  MultiAppDTKInterpolationTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppDTKInterpolationTransfer() {}

  virtual void execute();

protected:
  /**
   * Small helper function for finding the system containing the variable.
   *
   * Note that this implies that variable names are unique across all systems!
   *
   * @param es The EquationSystems object to be searched.
   * @param var_name The name of the variable you are looking for.
   */
  System * find_sys(EquationSystems & es, std::string & var_name);

  VariableName _from_var_name;
  AuxVariableName _to_var_name;

  DTKInterpolationHelper _helper;
  Point _master_position;
};

#endif /* MULTIAPPDTKINTERPOLATIONTRANSFER_H */

#endif //LIBMESH_HAVE_DTK
