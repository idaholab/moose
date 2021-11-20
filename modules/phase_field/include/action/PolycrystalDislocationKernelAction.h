/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#pragma once

#include "PolycrystalKernelAction.h"


/**
 * Action that adds the polycrystal dislocation deformation energy to the
 * grain growth problem.
 */
class PolycrystalDislocationKernelAction : public PolycrystalKernelAction
{
public:
  PolycrystalDislocationKernelAction(const InputParameters & params);

  static InputParameters validParams();

  virtual void act();

protected:
  /// name of the grain tracker that will be used with the dislocation energy kernels
  const std::string _grain_tracker_name;
};
