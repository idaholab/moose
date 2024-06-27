#pragma once
#include "auxsolver_base.h"
#include "coupled_coefficient_aux.h"
#include "curl_aux.h"
#include "helmholtz_projector.h"
#include "l2_error_vector_aux.h"
#include "scaled_curl_vector_gridfunction_aux.h"
#include "scaled_vector_gridfunction_aux.h"
#include "vector_coefficient_aux.h"
#include "vector_gridfunction_cross_product_aux.h"
#include "vector_gridfunction_dot_product_aux.h"
#include "flux_monitor_aux.h"

// Specify classes that perform auxiliary calculations on GridFunctions or
// Coefficients.
namespace platypus
{

class AuxSolvers : public platypus::NamedFieldsMap<platypus::AuxSolver>
{
private:
public:
  std::vector<std::pair<std::shared_ptr<platypus::AuxSolver>, std::string>> _aux_queue;
  void Init(const platypus::GridFunctions & gridfunctions, Coefficients & coefficients);
  void Solve(double t = 0.0);
};

} // namespace platypus
