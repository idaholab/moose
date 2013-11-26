#ifndef MULTIAPPPROJECTIONTRANSFER_H
#define MULTIAPPPROJECTIONTRANSFER_H

#include "MultiAppTransfer.h"
#include "libmesh/linear_implicit_system.h"

class MultiAppProjectionTransfer;

template<>
InputParameters validParams<MultiAppProjectionTransfer>();

/**
 * Project values from one domain to another
 */
class MultiAppProjectionTransfer : public MultiAppTransfer
{
public:
  MultiAppProjectionTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppProjectionTransfer();

  virtual void execute();

protected:
  void toMultiApp();
  void fromMultiApp();

  void assembleL2From(EquationSystems & es, const std::string & system_name);
  void assembleL2To(EquationSystems & es, const std::string & system_name);

  void projectSolution(FEProblem & fep, unsigned int app);

  AuxVariableName _to_var_name;
  VariableName _from_var_name;

  MooseEnum _proj_type;

  /// True, if we need to recompute the projection matrix
  bool _compute_matrix;
  std::vector<LinearImplicitSystem *> _proj_sys;
  /// Having one projection variable number seems weird, but there is always one variable in every system being used for projection,
  /// thus is always going to be 0 unless something changes in libMesh or we change the way we project variables
  unsigned int _proj_var_num;

  friend void assemble_l2_from(EquationSystems & es, const std::string & system_name);
  friend void assemble_l2_to(EquationSystems & es, const std::string & system_name);

};


#endif /* MULTIAPPPROJECTIONTRANSFER_H */
