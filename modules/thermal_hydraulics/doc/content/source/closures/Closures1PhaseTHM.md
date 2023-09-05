# Closures1PhaseTHM

!syntax description /Closures/Closures1PhaseTHM

The closures added are:

- a wall friction factor for the pressure drop,
- a wall heat transfer coefficient for heat transfer.

The user can choose between a range of correlations for the heat transfer coefficient and friction factor that covers  pipes to rod-bundles. The available correlations for the heat transfer coefficient are listed in [HTC-correlations].

!table id=HTC-correlations caption=Available correlations for the heat transfer coefficient.
| Name           | Coolant       | Geometry    | Reference                                                   |
| -------------- | ------------- | ----------- | ----------------------------------------------------------- |
| Dittus-Boelter | Liquid/Gases  | Pipes       | [ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial.md] |
| Gnielinski     | Liquid/Gases  | Pipes       | [ADWallHeatTransferCoefficientGnielinskiMaterial.md]        |
| Wolf-McCarthy  | Gases         | Pipes       | [ADWallHeatTransferCoefficientWolfMcCarthyMaterial.md]      |
| Lyon           | Liquid Sodium | Pipes       | [ADWallHeatTransferCoefficientLyonMaterial.md]              |
| Weisman        | Water         | Rod-bundles | [ADWallHeatTransferCoefficientWeismanMaterial.md]           |
| Kazimi-Carelli | Liquid Sodium | Rod-bundles | [ADWallHeatTransferCoefficientKazimiMaterial.md]            |
| Mikityuk       | Liquid Sodium | Rod-bundles | [ADWallHeatTransferCoefficientMikityukMaterial.md]          |
| Schad          | Liquid Sodium | Rod-bundles | [ADWallHeatTransferCoefficientSchadMaterial.md]             |

The available correlations for the friction factor are listed in [FF-correlations].

!table id=FF-correlations caption=Available correlations for the friction factor.
| Name          | Geometry    | Reference                            |
| ------------- | ----------- | ------------------------------------ |
| Churchill     | Pipes       | [ADWallFrictionChurchillMaterial.md] |
| Cheng-Todreas | Rod-bundles | [ADWallFrictionChengMaterial.md]     |


The user can also define the friction factor directly in the component block, overwriting the value given by the [Closures1PhaseTHM.md], for example:

```
[Components]
  [pipe_1]
    type = FlowChannel1Phase
    fp = water
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    f = 0.01
  []
  [pipe_2]
    type = FlowChannel1Phase
    fp = water
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
  []
[]
```

In this case, the friction factor for the `Pipe_1` component will be equal to the value defined in the parameter `f`, while for `Pipe_2` the friction factor will given by the correlation chosen in the [Closures1PhaseTHM.md].

Additionally, this object defines:

- a wall temperature material, to be able to retrieve the wall temperature as a material property for each heat transfer.

!syntax parameters /Closures/Closures1PhaseTHM

!syntax inputs /Closures/Closures1PhaseTHM

!syntax children /Closures/Closures1PhaseTHM
