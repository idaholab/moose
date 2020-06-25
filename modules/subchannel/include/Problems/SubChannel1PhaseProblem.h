#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "SubChannelMesh.h"

class SubChannel1PhaseProblem;

//template <>
//InputParameters validParams<SubChannel1PhaseProblem>();

/**
 * This is an interface to call my single phase sub channel Solver through
 an external problem interface
 */
class SubChannel1PhaseProblem : public ExternalProblem //This class inherits from ExternalProblem
{
public:
  SubChannel1PhaseProblem(const InputParameters & params); //Constructor
  ~SubChannel1PhaseProblem(); //Destructor
  static InputParameters validParams();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /*direction*/) override;
  virtual bool converged() override;

protected:
  //SubChannelMesh * _mesh;
  SubChannelMesh & _subchannel_mesh;

private:
  Real _mflux_in;
  Real _T_in;
  Real _P_out;
};
