/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COMPUTEINITIALCONDITIONS_H
#define COMPUTEINITIALCONDITIONS_H

namespace Moose
{
  /**
   * Sets the initial value of variables.
   */
  Number initial_value (const Point& p,
                        const Parameters& parameters,
                        const std::string& sys_name,
                        const std::string& var_name);

  /**
   * Sets the initial gradient of variables.
   */
  Gradient initial_gradient (const Point& p,
                             const Parameters& parameters,
                             const std::string& sys_name,
                             const std::string& var_name);

  /**
   * Function passed to libMesh for initial condition setting.
   */
  void initial_condition(EquationSystems& es, const std::string& system_name);
}

#endif //COMPUTEINITIALCONDITIONS_H
