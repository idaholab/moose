# FluidFlower International Benchmark Study

## Introduction

The FluidFlower International Benchmark Study was a collaborative effort at benchmarking numerical models of underground storate of CO$_2$ using experimental results obtained with the [FluidFlower](https://fluidflower.w.uib.no) experimental apparatus. The study was run by the University of Bergen, Norway, where the experiments were performed, with numerical modelling done by nine research groups worldwide.

## FluidFlower experimental apparatus

The FluidFlower is a large, curved Hele-Shaw like apparatus where detailed and realistic geological models can be created. Pressure sensors are located at regular intervals, and optical characterisation of gas saturation and dissolution enable detailed experiemntal characterisation of the physical processes involved in geological storage of CO$_2$.

## Numerical modelling study

The numerical models were developed without any access to the experimental results during the study. A full description of the problem is contained in [!citet](fluidflower). Participants initially developed their own models in the blind phase of the study, before comparing results amongst each other to enable groups to discuss important features. Finally, once each participant had submitted their final results, the experimental results were presented and compared to the numerical predictions. Full details are available in [!citet](flemisch2023).

## MOOSE model

The Porous Flow module of MOOSE was used to model the FluidFlower experiments. Full details of the modelling is avaialble in
[!citet](green2023), while a comparison to experiments is presented in [!citet](flemisch2023). As capillary barriers were a significant factor in the spatial distribution of CO$_2$, where gas saturation could be discontinuous between adjacent grid cells, the capability of the PorousFlow module was extended to use the finite volume method. This example serves as the first study using this new capability in the PorousFlow module.

A computational mesh was created using facies determined from high-resolution images of the exerimental apparatus, with petrophysical properties such as porosity and permeability assigned based on the values provided in [!citet](fluidflower), see [fluidflower_model]. Some important features to note are the (relatively) low permeability sealing units (dark blue facies), which also feature large capillary entry pressures. The lower sealing unit is bisected by a high permeability fault, so that CO$_2$ would be expected to breach the sealing unit through this pathway. A second high-permeability fault is located in the upper right part of the model, while an impermeable seal (the white region) seperates the upper part of the reservoir.

!media porous_flow/fluidflower_model.png
       style=width:80%;margin-left:10px
       caption=Permebility of each facies in the FluidFlower model
       id=fluidflower_model

The input file that can be used to reproduce the results presented in [!citet](green2023) using MOOSE is rather lengthy and complicated, due to the different properties in each facies, the injection schedule, and the quantities that were required to be reported as part of the study. One important thing to note is that this problem is quite challenging numerically due to the large density contrast between the gas and liquid phases at ambient pressure and temperature conditions, which meant that extremely small timesteps had to be used in order for the nonlinear solver to converge. Consequently, this problem takes a long time to run (on the order of a day), and as it is only a 2D model with O(40K) cells only, it doesn't benefit significantly from parallelisation.

Nonetheless, this example does show how it is possible to describe a complicated heterogeneous model in MOOSE. Parameter substitution through input file variables ([brace expresssions](input_syntax.md optional=True)) are used extensivley to avoid possible errors in values. These are described in the comments at the top of the input file.

!listing modules/porous_flow/examples/fluidflower/fluidflower.i end=Mesh


The rest of the input file is typical of a PorousFlow input file.

## Results

The MOOSE prediction of the spatial distribution of CO$_2$ in each phase over the five days of the FluidFlower experiment are shown in [fluidflower]. In this figure, bright yellow represents CO$_2$ in the gas phase, while lighter yellow represents CO$_2$ dissolved in the water. During the two injection phases, CO$_2$ is mainly in the gas phase. Due to the density contrast between the gas and liquid phases, the CO$_2$ rapidly migrates upwards until it reaches a sealing unit (where the capillary entry pressure is greater than the gas pressure). It then fills these structural traps until they are full. As the lower structural trap becomes full, excess CO$_2$ spills up the permeable fault in the bottom left of the model, migrating rapidly upwards until it reaches the next capillary barrier, where it spreads laterally again.

!media large_media/porous_flow/examples/fluidflower/fluidflower.mp4
       style=width:80%;margin-left:10px
       autoplay=True
       loop=True
       caption=CO$_2$ injection and dissolution in the FluidFlower geometry
       id=fluidflower

As time continues, CO$_2$ begins to dissolve into the water, increasing the liquid desnity slightly. This creates a gravitational instabilty, and results in convective mixing as evidenced by the complex downwelling fingers of CO$_2$ saturated water. Dissolution continues until there is no CO$_2$ in the gas phase at the end of the experiment.

Full details of the results obtained using MOOSE are presented in [!citet](green2023), while a comparison to the experimental results presented in [!citet](flemisch2023) show that the MOOSE predictions compared favourably to the experimental results.
