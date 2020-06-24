#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"

class SubChannel1PhaseProblem;

template <>
InputParameters validParams<SubChannel1PhaseProblem>();

/**
 * This is an interface to call my single phase sub channel Solver through
 an external problem interface
 */
class SubChannel1PhaseProblem : public ExternalProblem //This class inherits from ExternalProblem
{
public:
  SubChannel1PhaseProblem(const InputParameters & params); //Constructor
  ~SubChannel1PhaseProblem(); //Destructor

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /*direction*/) override;

protected:
  SubChannelMesh * _mesh;
  
private:
  Real _mflux_in
  Real _T_in
  Real _P_out
};
