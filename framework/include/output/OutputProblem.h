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

#ifndef OUTPUTPROBLEM_H
#define OUTPUTPROBLEM_H

#include "FEProblem.h"
#include "Output.h"
// libMesh
#include "equation_systems.h"
#include "vector_value.h"
#include "mesh_function.h"

class MooseMesh;
class OutputProblem;

template<>
InputParameters validParams<OutputProblem>();

class OutputProblem : public Problem
{
public:
  OutputProblem(const std::string & name, InputParameters parameters);
  virtual ~OutputProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual Problem * parent() { return NULL; }

  // Variables /////
  virtual bool hasVariable(const std::string & /*var_name*/) { return false; } // TODO
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) { return _mproblem.getVariable(tid, var_name); } // TODO
  virtual bool hasScalarVariable(const std::string & /*var_name*/) { return false; } // TODO
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name) { return _mproblem.getScalarVariable(tid, var_name); } // TODO

  virtual void subdomainSetup(SubdomainID /*subdomain*/, THREAD_ID /*tid*/) {}
  virtual void subdomainSetupSide(SubdomainID /*subdomain*/, THREAD_ID /*tid*/) {}

  virtual void prepare(const Elem * /*elem*/, THREAD_ID /*tid*/) {}
  virtual void prepare(const Elem * /*elem*/, unsigned int /*ivar*/, unsigned int /*jvar*/, const std::vector<unsigned int> & /*dof_indices*/, THREAD_ID /*tid*/) {}
  virtual void prepareAssembly(THREAD_ID /*tid*/) {}

  virtual void reinitElem(const Elem * /*elem*/, THREAD_ID /*tid*/) {}
  virtual void reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID /*tid*/) {}
  virtual void reinitNode(const Node * /*node*/, THREAD_ID /*tid*/) {}
  virtual void reinitNodeFace(const Node * /*node*/, BoundaryID /*bnd_id*/, THREAD_ID /*tid*/) {}
  virtual void reinitNodes(const std::vector<unsigned int> & /*nodes*/, THREAD_ID /*tid*/) { };
  virtual void reinitNeighbor(const Elem * /*elem*/, unsigned int /*side*/, THREAD_ID /*tid*/) {}
  virtual void reinitNeighborPhys(const Elem * /*neighbor*/, unsigned int /*neighbor_side*/, const std::vector<Point> & /*physical_points*/, THREAD_ID /*tid*/) {}
  virtual void reinitNodeNeighbor(const Node * /*node*/, THREAD_ID /*tid*/) {}
  virtual void reinitScalars(THREAD_ID /*tid*/) {}

  // Materials /////
  virtual void reinitMaterials(SubdomainID /*blk_id*/, THREAD_ID /*tid*/) {}
  virtual void reinitMaterialsFace(SubdomainID /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/) {}

  // Solve /////
  virtual void init();

  virtual bool computingInitialResidual() { return false; }

  virtual void computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, NumericVector<Number> & /*residual*/) {}
  virtual void computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, SparseMatrix<Number> & /*jacobian*/) {}
  virtual void computeBounds(NonlinearImplicitSystem & /*sys*/, NumericVector<Number> & /*lower*/, NumericVector<Number> & /*upper*/){}

  virtual void initialCondition(EquationSystems & /*es*/, const std::string & /*system_name*/) {}

  // Postprocessors /////
  virtual void computePostprocessors(ExecFlagType /*type*/ = EXEC_TIMESTEP) {}
  virtual void outputPostprocessors(bool /*force*/ = false) {}
  virtual Real & getPostprocessorValue(const std::string & /*name*/, THREAD_ID /*tid*/ = 0) { return _dummy; } // TODO

  // Function /////
  virtual void addFunction(std::string type, const std::string & name, InputParameters parameters) { mooseError("Can't add Functions to the output problem"); }
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0) { mooseError("Can't get Functions from the OutputProblem!"); }

  // Transient /////
  virtual void transient(bool /*trans*/) {}
  virtual bool isTransient() { return _mproblem.isTransient(); }

  virtual Real & time() { return _mproblem.time(); }
  virtual int & timeStep() { return _mproblem.timeStep(); }
  virtual Real & dt() { return _mproblem.dt(); }
  virtual Real & dtOld() { return _mproblem.dtOld(); }

  virtual void copySolutionsBackwards() {}

  // Setup /////
  virtual void timestepSetup();

  // Output system /////

  virtual Output & out() { return _out; }       // NOTE: don't like this -> remove and replace with better design
  virtual void output(bool /*force*/ = false) { _out.output(); }

  virtual void outputPps(const FormattedTable & table);
  virtual void outputInput();

protected:
  FEProblem & _mproblem;
  MooseMesh _mesh;
  EquationSystems _eq;
  Output _out;
  std::vector<std::vector<MeshFunction *> > _mesh_functions;
  NumericVector<Number> * _serialized_solution;

private:
  Real _dummy;
};

#endif
