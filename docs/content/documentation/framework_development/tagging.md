## Tagging System
In finite element calculations, for each element we need to compute physics mapping (from a reference element to a physics element), basis functions, derivatives of the basis,  materials and other element related information. This computation may be expensive, and it becomes even worse when multiple global vectors/matrices need to be filled because every single element has to be visited  multiple times. The basic idea of the tagging system is to resole this challenging issue. Every element is visited once only, and during the visit, all kernels (corresponding to PDE operators) are
evaluated. The local element residual/matrix for the current kernel will be added/inserted/cached to multiple targeted global vectors/matrices. Each kernel can contribute to multiple global vectors/matrices, and which vector/matrix the current kernel should contribute to is implemented by assigning tags. There are a few tags in a kernel, and the kernel calculation will accumulate local contribution to the global counterparts. Obviously, one global vector/matrix is contributed by multiple kernels as well.

## Design
All kernel-like objects inherit from [TaggingInterface](/TaggingInterface.md),  and they can be assigned to different tags through parameters *_vector_tags_* and *_matrix_tags_*. By default, "nontime" is set. The multiple local residuals/matrices are assembled by *_Assembly Object_* to their global counterparts that then are used in *_NonlinearSystem_* and *_FEPRoblem_*. Data flow is shown as follows:

!media media/framework/tagging/tagging_flow_chart.png width=50% padding-left=20px float=right caption = Tagging Flow Chart



In Kernel, we store multiple copies of the local residual/matrix as shown in the following code:

!listing id=combo caption=Local residuals.
```c++
void
Kernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}
```

In *_Assembly_*, we cache local residuals if the tagged vector does exist

!listing id=combo caption=Cache Residual.
```c++
void
Assembly::cacheResidual()
{
  auto & tag_name_to_tag_id = _sys.subproblem().getVectorTag();
  auto tag_it = tag_name_to_tag_id.begin();

  mooseAssert(tag_name_to_tag_id.size() == _sub_Re.size(),
              "the number of tags does not equal to the number of residuals ");

  const std::vector<MooseVariableFE *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
  {
    tag_it = tag_name_to_tag_id.begin();
    for (unsigned int i = 0; tag_it != tag_name_to_tag_id.end(); i++, ++tag_it)
      if (_sys.hasVector(tag_it->second))
        cacheResidualBlock(_cached_residual_values[i],
                           _cached_residual_rows[i],
                           _sub_Re[i][var->number()],
                           var->dofIndices(),
                           var->scalingFactor());
  }
}
```
Then, the cached residuals are added to global tagged vectors.  

!listing id=combo caption= Add Cached Residual.
```c++
void
Assembly::addCachedResiduals()
{
  auto & tag_name_to_tag_id = _sys.subproblem().getVectorTag();

  mooseAssert(tag_name_to_tag_id.size() == _sub_Re.size(),
              "the number of tags does not equal to the number of residuals ");

  for (auto tag_it = tag_name_to_tag_id.begin(); tag_it != tag_name_to_tag_id.end(); ++tag_it)
  {
    if (!_sys.hasVector(tag_it->second))
    {
      _cached_residual_values[tag_it->second].clear();
      _cached_residual_rows[tag_it->second].clear();
      continue;
    }
    addCachedResidual(_sys.getVector(tag_it->second), tag_it->second);
  }
}
```
