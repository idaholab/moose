#ifndef LAPLACEPROJECTIONTRANSFER_H
#define LAPLACEPROJECTIONTRANSFER_H

#include "Transfer.h"
#include "libmesh/linear_implicit_system.h"

class LaplaceProjectionTransfer;

template <>
InputParameters validParams<LaplaceProjectionTransfer>();

/**
 * Project values from one domain to another
 */
class LaplaceProjectionTransfer : public Transfer
{
public:
  LaplaceProjectionTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

protected:
  void assembleLaplaceProjection(EquationSystems & es, const std::string & system_name);

  void projectSolution();

  AuxVariableName _to_var_name;
  VariableName _from_var_name;

  LinearImplicitSystem * _proj_sys;
  /// Having one projection variable number seems weird, but there is always one variable in every system being used for projection,
  /// thus is always going to be 0 unless something changes in libMesh or we change the way we project variables
  unsigned int _proj_var_num;

  friend void assemble_laplace_proj(EquationSystems & es, const std::string & system_name);

  // These variables allow us to cache qps for fixed meshes.
  bool _fixed_meshes;
};

#endif /* LAPLACEPROJECTIONTRANSFER_H */
