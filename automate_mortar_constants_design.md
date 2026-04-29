# Automated Mortar Contact Scaling Parameters Design

**Issue:** #32856  
**Branch:** `automate-mortar-constants-32856`

## Motivation

The mortar-based frictional contact implementation in MOOSE requires two numerical scaling
parameters, `c_normal` and `c_tangential`, that must currently be set by trial and error. These
parameters appear in the primal-dual active set strategy (PDASS) complementarity constraints that
enforce normal and tangential contact conditions.

In `ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof`:
```cpp
const ADReal dof_residual = std::min(lm_value, weighted_gap * c);
```

In `ComputeFrictionalForceLMMechanicalContact::enforceConstraintOnDof{3d}`:
```cpp
const auto lambda_plus_cg    = contact_pressure + c   * weighted_gap;
lambda_t_plus_ctu[i]         = friction_lm_values[i]  + c_t * *tangential_vel[i] * _dt;
```

The intent of this design is to add an optional, fully automatic mode that computes both
parameters from the elasticity tensors of the contacting bodies — zero additional user input
required. The feature must be strictly opt-in and must not affect any existing simulation.

---

## Background: Physical Meaning of c_normal and c_tangential

**`c_normal`** (units: Pa/m = stiffness per unit volume) appears in `contact_pressure + c *
weighted_gap`. The weighted gap is area-integrated: `normalization_i` (the weighting function
norm stored alongside the gap) converts it back to a physical gap in meters. With
`normalize_c = true` the effective scaling becomes:

```
c_effective = c / normalization_i   [Pa/m]
```

For PDASS to be well-conditioned, `c_effective * gap_typical` must be comparable in magnitude
to `contact_pressure_typical`. Contact pressure scales as `E * gap / h` (Hertz-like), so:

```
c_effective ~ E / h   →   c = E   (when normalize_c handles the 1/h factor)
```

This is the standard result from mortar/penalty contact literature. The existing `normalize_c`
mechanism already provides the `1/h` factor; `c_normal` just needs to be on the order of the
material's Young's modulus.

**`c_tangential`** (same units) appears in `friction_lm + c_t * weighted_velocity * dt`. For
problems with steady sliding the same stiffness argument applies: `c_t = c_normal` is a
natural default. For problems where the sliding velocity varies widely, a per-node dynamic
adaptation (see §3) can improve convergence.

---

## Key Architectural Observation

`WeightedGapUserObject` is the base class for both LM-based contact UOs
(`LMWeightedGapUserObject`, `LMWeightedVelocitiesUserObject`) and penalty/cohesive-zone UOs
(`PenaltyWeightedGapUserObject`, `PenaltyFrictionUserObject`, `CohesiveZoneModelBase`). The
`c_normal` parameter only exists in LM-based mortar contact, so the new feature belongs in
`LMWeightedGapUserObject`, not the base class.

`WeightedGapUserObject` already inherits `TwoMaterialPropertyInterface`
(`WeightedGapUserObject.C:26`: `params += TwoMaterialPropertyInterface::validParams()`), so
`LMWeightedGapUserObject` can call `getMaterialProperty<RankFourTensor>("elasticity_tensor")`
for the secondary side and `getNeighborMaterialProperty<RankFourTensor>("elasticity_tensor")`
for the primary side — with no new infrastructure.

The contact normal `_normals[_i]` is already available in `computeQpIProperties()` — it is
used there to project the gap vector onto the surface normal. This is the right place to
accumulate the normal-direction stiffness: the acoustic tensor contraction
`n_i n_j C_ijkl n_k n_l` uses the same per-node normal and the same test-function weight
`(*_test)[_i][_qp] * _qp_factor` that already accumulates the gap normalization, keeping
the two quantities on a consistent basis. The result is reduced across processors in
`finalize()` alongside the existing gap communication.

---

## Proposed Changes

### 1. New `ContactAction` Parameters

Add one new parameter to `ContactAction::validParams()` alongside the existing `c_normal`
and `c_tangential` declarations (currently lines 192–201):

| Parameter | Type | Default | Purpose |
|-----------|------|---------|---------|
| `c_normal_strategy` | `MooseEnum` | `"user"` | `user` (current behavior) or `physical` (fully automatic) |
| `c_tangential_strategy` | `MooseEnum` | `"user"` | `user` (current behavior) or `physical` (velocity-scaled, linked to `c_normal_eff`) |
| `secondary_elasticity_tensor_base_name` | `std::string` | `""` | Base name prefix of the elasticity tensor property on the secondary body; only used when `c_normal_strategy = physical` |
| `primary_elasticity_tensor_base_name` | `std::string` | `""` | Base name prefix of the elasticity tensor property on the primary body; only used when `c_normal_strategy = physical` |
| `elasticity_tensor_is_ad` | `bool` | `false` | Set to true if the elasticity tensor material property was declared as AD; only used when `c_normal_strategy = physical` |

When `c_normal_strategy = physical`:
- `ContactAction` emits a `paramError` if `c_normal` was explicitly set by the user
  (`isParamSetByUser("c_normal")`). The two options are mutually exclusive: `physical` means
  the value is derived from the material system and no manual override is meaningful.
- `ContactAction` sets `derive_c_from_elasticity = true` on the weighted gap UserObject it
  already creates (passing through `params.set<bool>`).
- `ContactAction` forces `normalize_c = true` on the downstream constraint objects.

When `c_tangential_strategy = physical`:
- Only valid for Coulomb friction mortar; `paramError` otherwise.
- `c_t` is derived per-node from `c_normal_eff` with dynamic velocity scaling: `c_t = c_normal_eff / (vt_mag * dt)` during sliding, falling back to `c_normal_eff` during stick (see §3).
- `ContactAction` sets `dynamic_c_t = true` on the frictional constraint.

All new parameters are mortar-only, consistent with the existing validation for `c_normal`
(lines 387–392).

### 2. Automatic Effective Modulus in `LMWeightedGapUserObject`

**New parameters** (added to `LMWeightedGapUserObject::validParams()`):
```cpp
params.addParam<bool>("derive_c_from_elasticity", false,
    "Compute an effective elastic modulus from contact surface material properties "
    "for use as the c_normal constraint parameter.");
params.addParam<std::string>("secondary_base_name", "",
    "Base name prefix of the elasticity_tensor material property on the secondary body. "
    "Must match the base_name used on the secondary material block that computes the "
    "elasticity tensor.");
params.addParam<std::string>("primary_base_name", "",
    "Base name prefix of the elasticity_tensor material property on the primary body. "
    "Must match the base_name used on the primary material block that computes the "
    "elasticity tensor.");
params.addParam<bool>("elasticity_tensor_is_ad", false,
    "Whether the elasticity tensor material property was declared as an AD property. "
    "Set to true if the material block uses an AD elasticity tensor.");
```

**Implementation note — `base_name` and property lookup:**  
`ComputeElasticityTensorBase` (the base class for standard MOOSE elasticity tensor materials)
declares its property as `_base_name + "elasticity_tensor"`, where `_base_name` is either
empty or `"<prefix>_"`. The UO must construct the same name:
```cpp
const std::string sec_base = getParam<std::string>("secondary_base_name");
const std::string pri_base = getParam<std::string>("primary_base_name");
const std::string sec_name = sec_base.empty() ? "elasticity_tensor" : sec_base + "_elasticity_tensor";
const std::string pri_name = pri_base.empty() ? "elasticity_tensor" : pri_base + "_elasticity_tensor";
// dispatches to the template helper described under "New members" below
if (_elasticity_tensor_is_ad)
  fetchElasticityTensorProperties<true>(sec_name, pri_name);
else
  fetchElasticityTensorProperties<false>(sec_name, pri_name);
```

`ContactAction` does not currently have a `base_name` parameter. Three new optional
parameters, `secondary_elasticity_tensor_base_name`, `primary_elasticity_tensor_base_name`,
and `elasticity_tensor_is_ad`, should be added to `ContactAction` and forwarded to the UO
when `derive_c_from_elasticity = true`. The secondary and primary bodies may have different
`base_name` prefixes on their respective elasticity tensor materials, so a single shared name
is insufficient. This keeps the user from having to set these directly on the UO, which is
an implementation detail they should not need to know about.

**New members** (added to `LMWeightedGapUserObject.h`):
```cpp
const bool _derive_c_from_elasticity;
const bool _elasticity_tensor_is_ad;
// Exactly one pointer per side is non-null depending on _elasticity_tensor_is_ad
const MaterialProperty<RankFourTensor>   * _elasticity_tensor_secondary    = nullptr;
const MaterialProperty<RankFourTensor>   * _elasticity_tensor_primary      = nullptr;
const ADMaterialProperty<RankFourTensor> * _elasticity_tensor_secondary_ad = nullptr;
const ADMaterialProperty<RankFourTensor> * _elasticity_tensor_primary_ad   = nullptr;
// Per-node derived c values; pair is {accumulated C_nn * weight, accumulated weight}
// mirrors the layout of _dof_to_weighted_gap
std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_derived_c;
```

Mirroring `_dof_to_weighted_gap`, accumulation is per-node so each entry carries its own
AD derivative information. A private template helper is the only place that calls
`getGenericMaterialProperty` / `getGenericNeighborMaterialProperty`:
```cpp
template <bool is_ad>
void fetchElasticityTensorProperties(const std::string & sec_name,
                                     const std::string & pri_name)
{
  if constexpr (is_ad)
  {
    _elasticity_tensor_secondary_ad = &getGenericMaterialProperty<RankFourTensor, true>(sec_name);
    _elasticity_tensor_primary_ad   = &getGenericNeighborMaterialProperty<RankFourTensor, true>(pri_name);
  }
  else
  {
    _elasticity_tensor_secondary = &getGenericMaterialProperty<RankFourTensor, false>(sec_name);
    _elasticity_tensor_primary   = &getGenericNeighborMaterialProperty<RankFourTensor, false>(pri_name);
  }
}
```

Called in the constructor as:
```cpp
if (_elasticity_tensor_is_ad)
  fetchElasticityTensorProperties<true>(sec_name, pri_name);
else
  fetchElasticityTensorProperties<false>(sec_name, pri_name);
```

In `computeQpIProperties`, dispatch to a second template helper so each instantiation
has a single consistent type and `auto` deduces correctly within it:
```cpp
// Called from computeQpIProperties:
if (_elasticity_tensor_is_ad)
  accumulateDerivedC<true>();
else
  accumulateDerivedC<false>();
```

```cpp
template <bool is_ad>
void LMWeightedGapUserObject::accumulateDerivedC()
{
  const auto & sec_prop = [&]() -> const auto &
  {
    if constexpr (is_ad) return *_elasticity_tensor_secondary_ad;
    else                 return *_elasticity_tensor_secondary;
  }();
  const auto & pri_prop = [&]() -> const auto &
  {
    if constexpr (is_ad) return *_elasticity_tensor_primary_ad;
    else                 return *_elasticity_tensor_primary;
  }();

  const RealVectorValue & n = _normals[_i];
  auto C_nn_sec = decltype(sec_prop[_qp](0,0,0,0)){0};
  auto C_nn_pri = decltype(pri_prop[_qp](0,0,0,0)){0};
  for (const auto a : make_range(3))
    for (const auto b : make_range(3))
      for (const auto c : make_range(3))
        for (const auto d : make_range(3))
        {
          const auto w = n(a) * n(b) * n(c) * n(d);
          C_nn_sec += w * sec_prop[_qp](a, b, c, d);
          C_nn_pri += w * pri_prop[_qp](a, b, c, d);
        }
  const auto C_nn_harm = 2.0 * C_nn_sec * C_nn_pri / (C_nn_sec + C_nn_pri);
  const auto test_weight = (*_test)[_i][_qp] * _qp_factor;
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));
  auto & [c_num, c_denom] = _dof_to_derived_c[dof];
  c_num   += C_nn_harm * test_weight;
  c_denom += test_weight;
}
```

In `finalize()`, after communicating gaps, divide `c_num` by `c_denom` for each node
(and perform the parallel reduction analogous to `communicateGaps`), then expose via:
```cpp
const std::unordered_map<const DofObject *, std::pair<ADReal, Real>> & dofToDerivedC() const;
```

After `finalize()`, `c_num / c_denom` is the test-function-weighted average of `C_nn` over
the mortar segments touching that node — a representative elastic stiffness at that node in
units of Pa. This normalization is internal to the UO and is purely about averaging the
material property across quadrature points; it has nothing to do with element size.

The constraint then reads `c_nn` (the first pair element, already averaged) and optionally
applies `normalize_c`, which is a separate and independent operation:
```cpp
const auto & [c_nn, _] = libmesh_map_find(_weighted_gap_uo.dofToDerivedC(), dof_object);
const ADReal c = _normalize_c ? c_nn / *_normalization_ptr : c_nn;
```

`normalize_c` divides by `*_normalization_ptr`, the weighting function norm for that node
(units of area). This accounts for the fact that the weighted gap is area-integrated: without
it, `c` would have an implicit element-size dependence. The two normalizations are therefore
independent: the UO's `c_denom` converts accumulated C_nn integrals into a material stiffness
value; `normalize_c` in the constraint converts that stiffness value into the correct scale
relative to the area-integrated gap.

No `raw_value` is needed anywhere: AD derivatives propagate from the elasticity tensor
through the acoustic tensor contraction into the constraint residual and Jacobian.

The material properties are only retrieved in the constructor when `_derive_c_from_elasticity`
is true, so there is zero overhead for the default path.

The acoustic tensor contraction `n_i n_j C_ijkl n_k n_l` gives the normal-direction stiffness
for any elasticity tensor — isotropic, orthotropic, or fully anisotropic — without requiring
extraction of scalar moduli. For isotropic materials it equals λ+2μ regardless of the contact
normal direction.

**In `LMWeightedGapUserObject::initialize()`**: clear `_dof_to_derived_c`.

**In `LMWeightedGapUserObject::finalize()`** (after calling `WeightedGapUserObject::finalize()`):
communicate the per-node map across processors (analogous to `communicateGaps`), then
normalize each entry:
```cpp
for (auto & [dof, c_pr] : _dof_to_derived_c)
  c_pr.first /= c_pr.second;
```

### 3. Constraint Changes

#### Normal constraint (`ComputeWeightedGapLMMechanicalContact`)

The constraint already holds a reference to `_weighted_gap_uo`. In physical mode the
per-node `c` is read directly from `dofToDerivedC()` during `enforceConstraintOnDof` rather
than from the scalar `_c` member, so `_c` is bypassed entirely for this code path.

**In `ContactAction::act()`** (inside the mortar setup block, near line 1072):
```cpp
if (getParam<MooseEnum>("c_normal_strategy") == "physical")
{
  if (isParamSetByUser("c_normal"))
    paramError("c_normal",
               "Cannot set 'c_normal' when 'c_normal_strategy = physical'; "
               "the value is derived from the material elasticity tensor.");
  params.set<bool>("normalize_c") = true;
  params.set<bool>("use_derived_c_normal") = true;
}
else
  params.set<Real>("c") = getParam<Real>("c_normal");
```

Add a `use_derived_c_normal` bool to `ComputeWeightedGapLMMechanicalContact`. When set,
`enforceConstraintOnDof` looks up `c_nn` from `_weighted_gap_uo.dofToDerivedC()` instead
of using `_c`, as shown in the constraint snippet in §2 above.

#### Tangential constraint (`ComputeFrictionalForceLMMechanicalContact`)

**Tightening UO member types in the constraint classes**

The existing `_weighted_gap_uo` and `_weighted_velocities_uo` members in
`ComputeWeightedGapLMMechanicalContact` and `ComputeFrictionalForceLMMechanicalContact` are
currently typed as the base classes `WeightedGapUserObject` and `WeightedVelocitiesUserObject`.
Inspection of the full test suite shows that every test using `ComputeFrictionalForceLMMechanicalContact`
pairs it exclusively with `LMWeightedVelocitiesUserObject`; no test uses a non-LM UO
(e.g. `PenaltyFrictionUserObject`) with this constraint. The broad base-class typing is
therefore over-generalization with no demonstrated need.

As part of this change, retype both members to their LM-specific derived classes:

- `ComputeWeightedGapLMMechanicalContact`: `_weighted_gap_uo` → `const LMWeightedGapUserObject &`
- `ComputeFrictionalForceLMMechanicalContact`: `_weighted_velocities_uo` → `const LMWeightedVelocitiesUserObject &`

With `_weighted_gap_uo` already typed as `LMWeightedGapUserObject`, `dofToDerivedC()` is
directly accessible with no downcast needed. In `enforceConstraintOnDof`, `c` is resolved
per-node as:
```cpp
ADReal c;
if (_use_derived_c_normal)
{
  const auto & [c_nn, _] = libmesh_map_find(_weighted_gap_uo.dofToDerivedC(), dof_object);
  c = _normalize_c ? c_nn / *_normalization_ptr : c_nn;
}
else
  c = _normalize_c ? _c / *_normalization_ptr : _c;
```

The existing `Real _c` member is used unchanged in the `user` strategy path.

**`physical` c_t computation**

The `physical` strategy requires per-node `c_t` in `enforceConstraintOnDof{,3d}`.
Add:

```cpp
const bool _dynamic_c_t;
const Real _vel_floor;  // fallback threshold: below this velocity magnitude, use c_normal_eff
```

`c_t` is `ADReal` throughout so that derivatives from the normal stiffness (in physical mode)
and from the tangential velocity (already `ADReal`) propagate into the Jacobian. The normal
reference stiffness is resolved by the same two-path logic used in the normal constraint:
```cpp
// Resolve normal reference stiffness (physical or user mode)
ADReal c_normal_eff;
if (_use_derived_c_normal)
{
  const auto & [c_nn, _] = libmesh_map_find(_weighted_velocities_uo.dofToDerivedC(), dof_object);
  c_normal_eff = _normalize_c ? c_nn / *_normalization_ptr : c_nn;
}
else
  c_normal_eff = _normalize_c ? _c / *_normalization_ptr : _c;

ADReal c_t;
if (_dynamic_c_t)
{
  // In 3D use the full tangential velocity magnitude to avoid directional bias
  const auto vt_sq = *_tangential_vel_ptr[0] * *_tangential_vel_ptr[0] + 1e-48;
  const auto vt_mag = std::sqrt(vt_sq);  // 3D variant adds vt_1^2 + vt_2^2
  if (MetaPhysicL::raw_value(vt_mag) < _vel_floor)
    c_t = c_normal_eff;  // sticking: fall back to normal stiffness
  else
    c_t = c_normal_eff / (vt_mag * _dt);
}
else
  c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;
```

`c_normal_eff` serves as the reference stiffness so that `c_t * v_t * dt ~ c * gap` when
`v_t * dt ~ gap` — keeping both constraint terms on the same scale. During stick
(`vt_mag < _vel_floor`), `c_t` falls back to `c_normal_eff` rather than blowing up.

`ContactAction` sets `dynamic_c_t = true` on the frictional constraint when
`c_tangential_strategy == physical`.

### 4. Files to Create or Modify

| File | Change |
|------|--------|
| `modules/contact/src/actions/ContactAction.C` | New enum params; physical-mode logic; pass `derive_c_from_elasticity` / `use_derived_c_normal` / `dynamic_c_t` flags |
| `modules/contact/include/userobjects/LMWeightedGapUserObject.h` | New members + `derivedCNormal()` accessor |
| `modules/contact/src/userobjects/LMWeightedGapUserObject.C` | Elasticity tensor reads in `computeQpIProperties`; accumulation + reduction in `initialize`/`finalize` |
| `modules/contact/include/constraints/ComputeWeightedGapLMMechanicalContact.h` | `_use_derived_c_normal` bool; retype `_weighted_gap_uo` from `WeightedGapUserObject` to `LMWeightedGapUserObject` |
| `modules/contact/src/constraints/ComputeWeightedGapLMMechanicalContact.C` | `enforceConstraintOnDof` reads per-node `c_nn` from `_weighted_gap_uo.dofToDerivedC()` when `_use_derived_c_normal` is set |
| `modules/contact/include/constraints/ComputeFrictionalForceLMMechanicalContact.h` | `_dynamic_c_t`, `_vel_floor` members; retype `_weighted_velocities_uo` from `WeightedVelocitiesUserObject` to `LMWeightedVelocitiesUserObject` |
| `modules/contact/src/constraints/ComputeFrictionalForceLMMechanicalContact.C` | Dynamic `ADReal c_t` in `enforceConstraintOnDof` and `enforceConstraintOnDof3d`; call `_weighted_velocities_uo.dofToDerivedC()` for physical-mode `_c` |
| `modules/contact/test/tests/physical_mortar_constants/` | New test directory (see §5) |

No new files outside `contact/` are required.

---

## Backward Compatibility

- Both enums default to `"user"`, so all existing input files are unaffected.
- `c_normal` and `c_tangential` parameters remain in `validParams()` with their existing
  defaults (1e6 and 1). They are used unchanged when the corresponding strategy is `"user"`.
- `derive_c_from_elasticity` defaults to `false` on `LMWeightedGapUserObject`, so the material
  property reads and accumulation code are never reached for existing problems.

---

## Testing Plan

### New Tests

1. **`physical_mortar_constants/frictionless_physical.i`**  
   Single-material Hertz contact geometry. Set `c_normal_strategy = physical`. Verify
   convergence and that the resulting contact pressure matches a reference run that uses the
   manually-computed equivalent `c_normal` (i.e., `E_material` with `normalize_c = true`).

2. **`physical_mortar_constants/bimaterial_physical.i`**  
   Two-body contact with different moduli (e.g., steel on softer polymer). Confirm the harmonic
   mean is used and that the simulation converges without user-tuned parameters.

3. **`physical_mortar_constants/frictional_physical.i`**  
   Sliding block with nonzero sliding velocity. Set `c_tangential_strategy = physical`.
   Verify convergence and check that nonlinear iteration count is equal to or lower than a
   reference run with fixed `c_t`.

4. **Parameter validation tests**  
   - `c_tangential_strategy = physical` on a frictionless contact pair → `paramError`
   - `c_normal_strategy = physical` with `c_normal` explicitly set by the user → `paramError`

### Existing Test Suite

Run the full `modules/contact` test suite. All existing tests use fixed `c_normal`/`c_tangential`
values (strategy = `"user"`) and must produce identical results.

