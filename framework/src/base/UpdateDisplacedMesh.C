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
  UpdateDisplacedMesh(MooseSystem &sys, const std::vector<Number>& in_soln)
    :_moose_system(sys),
     _soln(in_soln)
  {}

  void operator() (const NodeRange & range) const
  {
    ParallelUniqueId puid;
    unsigned int tid = puid.id;

    std::vector<std::string> displacement_variables = _moose_system.getDisplacementVariables();
    unsigned int num_displacements = displacement_variables.size();
    std::vector<unsigned int> var_nums(num_displacements);

    for(unsigned int i=0; i<num_displacements; i++)
      var_nums[i] = _moose_system.getVariableNumber(displacement_variables[i]);

    Mesh * reference_mesh = _moose_system.getMesh();
    
    unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();

    NodeRange::const_iterator nd = range.begin();

    for (nd = range.begin() ; nd != range.end(); ++nd)
    {
      Node & displaced_node = *(*nd);
      
      Node & reference_node = reference_mesh->node(displaced_node.id());

      for(unsigned int i=0; i<num_displacements; i++)
        displaced_node(i) = reference_node(i) + _soln[reference_node.dof_number(nonlinear_system_number, var_nums[i], 0)];
    }
  }

protected:
  MooseSystem &_moose_system;
  const std::vector<Number> & _soln;
};

void MooseSystem::updateDisplacedMesh(const NumericVector<Number>& soln)
{
  Moose::perf_log.push("updateDisplacedMesh()","Solve");

  std::vector<Number> localized_solution;
  
  soln.localize(localized_solution);

  Threads::parallel_for(*getActiveNodeRange(),
                        UpdateDisplacedMesh(*this, localized_solution));
  
  Moose::perf_log.pop("updateDisplacedMesh()","Solve");
}

