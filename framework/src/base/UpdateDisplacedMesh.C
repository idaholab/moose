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

//Moose Includes
#include "Moose.h"
#include "MaterialFactory.h"
#include "BoundaryCondition.h"
#include "ParallelUniqueId.h"
#include "MooseSystem.h"
#include "ElementData.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_vector.h"
#include "petsc_matrix.h"
#include "dof_map.h"
#include "mesh.h"
#include "boundary_info.h"
#include "elem_range.h"

#include <vector>

class UpdateDisplacedMesh
{
public:
  UpdateDisplacedMesh(MooseSystem &sys, const NumericVector<Number>& in_soln, const NumericVector<Number>& in_aux_soln)
    :_moose_system(sys),
     _soln(in_soln),
     _aux_soln(in_aux_soln)
  {}

  void operator() (const NodeRange & range) const
  {
    ParallelUniqueId puid;

    std::vector<std::string> displacement_variables = _moose_system.getDisplacementVariables();
    unsigned int num_displacements = displacement_variables.size();
    
    std::vector<unsigned int> var_nums;
    std::vector<unsigned int> var_nums_directions;
    
    std::vector<unsigned int> aux_var_nums;
    std::vector<unsigned int> aux_var_nums_directions;

    for(unsigned int i=0; i<num_displacements; i++)
    {
      std::string displacement_name = displacement_variables[i];
      
      if(_moose_system.hasVariable(displacement_name))
      {
         var_nums.push_back(_moose_system.getVariableNumber(displacement_name));
         var_nums_directions.push_back(i);
      }
      else if(_moose_system.hasAuxVariable(displacement_name))
      {
         aux_var_nums.push_back(_moose_system.getAuxVariableNumber(displacement_name));
         aux_var_nums_directions.push_back(i);
      }
      else
        mooseError("Undefined variable used for displacements!");
    }

    unsigned int num_var_nums = var_nums.size();
    unsigned int num_aux_var_nums = aux_var_nums.size();

    Mesh * reference_mesh = _moose_system.getMesh();
    
    unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();
    unsigned int aux_system_number = _moose_system.getAuxSystem()->number();

    NodeRange::const_iterator nd = range.begin();

    for (nd = range.begin() ; nd != range.end(); ++nd)
    {
      Node & displaced_node = *(*nd);
      
      Node & reference_node = reference_mesh->node(displaced_node.id());
      
      for(unsigned int i=0; i<num_var_nums; i++)
      {
        unsigned int direction = var_nums_directions[i];
        displaced_node(direction) = reference_node(direction) + _soln(reference_node.dof_number(nonlinear_system_number, var_nums[i], 0));
      }

      for(unsigned int i=0; i<num_aux_var_nums; i++)
      {
        unsigned int direction = aux_var_nums_directions[i];
        displaced_node(direction) = reference_node(direction) + _aux_soln(reference_node.dof_number(aux_system_number, aux_var_nums[i], 0));
      }
    }
  }

protected:
  MooseSystem &_moose_system;
  const NumericVector<Number> & _soln;
  const NumericVector<Number> & _aux_soln;
};

void MooseSystem::updateDisplacedMesh(const NumericVector<Number>& soln)
{
  Moose::perf_log.push("updateDisplacedMesh()","Solve");

  (*_displaced_system->solution) = soln;  
  (*_displaced_aux_system->solution) = *_aux_system->solution;

  Threads::parallel_for(*getActiveNodeRange(),
                        UpdateDisplacedMesh(*this, _serialized_solution, _serialized_aux_solution));
  
  Moose::perf_log.pop("updateDisplacedMesh()","Solve");
}

