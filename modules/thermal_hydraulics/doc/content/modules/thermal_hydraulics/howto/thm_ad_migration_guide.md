# How to Update Input Files for Automated Differentiation (AD)


## Components

There are +no changes+ required in the `[Components]` block.

## Materials

If you are using `[Materials]` block in your input file, refer to [material-property-flow] and [material-property-hs] for which material properties became AD and which stayed non-AD.
If your object is using an AD material property, you will need to change its type according to [material-type] to make it work with AD.

!table id=material-property-flow caption=List of AD and non-AD material properties related to 1-phase flow
| Property name | AD? |
| :- | :- |
| D_h | NO |
| direction | NO |
| c | YES |
| cp | YES |
| cv | YES |
| e | YES |
| h | YES |
| H | YES |
| k | YES |
| Re | YES |
| rho | YES |
| rhoA | YES |
| rhouA | YES |
| rhoEA | YES |
| p | YES |
| Pr | YES |
| q_wall | YES |
| T | YES |
| T_wall | YES |
| v | YES |
| vel | YES |

!table id=material-property-hs caption=List of AD and non-AD material properties related to heat conduction
| Property name | AD? |
| :- | :- |
| density | YES |
| thermal_conductivity | YES |
| specific_heat | YES |


!table id=material-type caption=Corresponding types of non-AD and AD materials.
| non-AD type | AD type |
| :- | :- |
| ConstantMaterial | ADConstantMaterial |
| GenericConstantMaterial | ADGenericConstantMaterial |
| ParsedMaterial | ADParsedMaterial |
| PrandtlNumberMaterial | ADPrandtlNumberMaterial |
| ReynoldsNumberMaterial | ADReynoldsNumberMaterial |
| WallHeatTransferCoefficient3EqnDittusBoelterMaterial | ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial |
| WallFrictionChurchillMaterial | ADWallFrictionChurchillMaterial |



## Postprocessors

If you are using `[Postprocessors]` block in your input file, use [pps-type] to convert the types of postprocessors to AD.

!table id=pps-type caption=Corresponding types of non-AD and AD postprocessors.
| non-AD type | AD type |
| :- | :- |
| ElementIntegralMaterialProperty | ADElementIntegralMaterialProperty |
| ElementAverageMaterialProperty | ADElementAverageMaterialProperty |
| ElementExtremeMaterialProperty | ADElementExtremeMaterialProperty |
| FlowBoundaryFlux1Phase | ADFlowBoundaryFlux1Phase |
| FlowJunctionFlux1Phase | ADFlowJunctionFlux1Phase |
| HeatRateConvection1Phase | ADHeatRateConvection1Phase |
| HeatStructureEnergy | ADHeatStructureEnergy |
| HeatStructureEnergy3D | ADHeatStructureEnergy3D |
| HeatStructureEnergyRZ | ADHeatStructureEnergyRZ |
| SideFluxIntegralRZ | ADSideFluxIntegralRZ |
| SpecificImpulse1Phase | ADSpecificImpulse1Phase |

## VectorPostprocessors

If you are using `[VectorPostprocessors]` block in your input file, use [vpps-type] to convert the types of vector postprocessors to AD.

!table id=vpps-type caption=Corresponding types of non-AD and AD vetcor postprocessors.
| non-AD type | AD type |
| :- | :- |
| Sampler1DReal | ADSampler1DReal |


## Time Integrators

Use [time-int-type] to convert time integrators.
The changes are needed for explicit integration.
Implicit integration does not need any changes, but it is listed for completeness.
If `type` is given in the table, you will need to use the time integrator block in your input file like so:

```
[TimeIntegrator]
  type = ...
  other params
[]
```

!table id=time-int-type caption=Time integrator conversion.
| Time integrator | How to update |
| :- | :- |
| implicit-euler | No change |
| bdf-2 | No change |
| explicit-euler | type = ActuallyExplicitEuler |
| explicit-tvd-rk-2 | type = ExplicitSSPRungeKutta, order = 2 |
| explicit-midpoint | type = ExplicitSSPRungeKutta, order = 2 |


## Executioner

With your input file converted into AD, make sure that you set `solve_type = NEWTON` in the `[Executioner]` block.


## Custom Closures

Custom closure are closure correlations specified on input file level.
The following example shows how to update an input file using those:

Original input file

```
[Materials]
  [f_wall_mat]
    type = GenericConstantMaterial
    block = 'pipe'
    prop_names = 'f_D df_D/drhoA df_D/drhouA df_D/drhoEA'
    prop_values = '0.123 0 0 0'
  []
[]
```

New input file

```
[Materials]
  [f_wall_mat]
    type = ADGenericConstantMaterial
    block = 'pipe'
    prop_names = 'f_D'
    prop_values = '0.123'
  []
[]
```

Note that with AD, users no longer have to provide `dX/dY` properties.


## Common errors related AD

If you see any of the following errors, it may indicate that the input file is running non-AD setup.
If the above information does not help you to fix the problem, contact THM developers.

```
*** ERROR ***
The requested regular material property thermal_conductivity is declared as an AD property.
Either retrieve it as an AD property with getADMaterialProperty or declare it as a regular
property with declareProperty
```
