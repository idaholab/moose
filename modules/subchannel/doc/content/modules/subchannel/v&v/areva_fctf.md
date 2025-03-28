# Deformed duct validation

*Contact: Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov*

## Validation overview

This work introduces SCM validation for the implemented capability to model flows within a deformed duct. The deformed geometry considered in this paper, has deformations of the kind that have been observed in LMFBR fuel assemblies. For this purpose, this work leverages experimental data produced in the context of the project: Thermal Hydraulic Computational Fluid Dynamics Simulations and Experimental Investigation of Deformed Fuel Assemblies [!citep](mays2017thermal). This project was a two-year collaboration between AREVA, TerraPower, Argonne National Laboratory (ANL) and Texas A\&M University (TAMU). Experiments were performed by AREVA and TAMU. Numerical simulations of these experiments were performed by TerraPower and ANL. The experimental data used in this work were produced during the second phase (objectives 2.3, 2.4) of the project:Non-Dilated/Dilated Duct Heated Bundle Experiment. The work presented here is a summary of the validation that is documented in [!citep](kyriakopoulos2024validation). For more details on the validation the reader is encouraged to access the cited document.

The project experiments included the first known wire-wrapped assembly experiment with deformed duct geometries and the largest numerical simulations ever produced for wire-wrapped bundles. The specific bundle designs are not exact designs of specific LMFBR fuel assemblies, but are representative in order to acquire data for use in validation. AREVA and TerraPower cooperated to produce a deformed and non-deformed duct test section bundle, which was integrated in the Areva fuel cooling test facility (FCTF). Both experiment bundles incorporated a 61 wire-wrapped pin bundle design. 61 pins were chosen because 61 pins results in a large enough of a bundle, in terms of rows of pins in the hexagonal packing configuration to minimize wall effects and therefore, represent a full size bundle while also limiting the computational resources and experiment costs.

## Experimental specifications

The FCTF in Richland, Washington, at the AREVA fuel fabrication facility was modified to accommodate the wire wrapped heated pin assembly test section. Steady state thermal-hydraulic tests were run with water flowing through the bundle at a Reynolds number $\approx 20,000$ in the fully turbulent regime. The bundle was partially heated: 19 of the 61 pins were electric heater rods that provided heating. Measurements included thermocouples at the unheated tube walls and fluid interface, embedded inside heaters, and on the internal duct wall. Pressure drop measurements were taken across different lengths of the pin bundle to capture entrance effects, planar pressure gradient due to bulk flow swirl, and fully developed conditions.

The deformed duct was a hexagonal duct where the inner duct walls had an axial and lateral (between duct corners for each duct face) profile shape, where the hexagonal flow area expanded from the nominal size and then contracted to return to the nominal hexagonal shape along the axial length. This bundle represents a later-in-life duct geometry. The deformed duct did not deform in the experiments, but instead was machined to represent the expected shape of a deformed duct due to conditions in the reactor. A picture of the deformed and the non-deformed duct is shown in [duct]. An illustration of the deformation, its functional form, and other geometric details of the deformed heated bundle are shown in [bundle].

!media subchannel/v&v/duct.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=duct
       caption=Cross-sections of experimental fuel pin bundle [!citep](mays2017thermal).

!media subchannel/v&v/bundle.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=bundle
       caption=Illustration (not to scale) of the deformed heated bundle [!citep](mays2017thermal).

The experiments were conducted at the FCTF. The facility is a high-temperature test facility that provides a maximum 750 kW power with distribution control to produce unique radial heater power peaking. The deformed heated bundle has the same geometric properties as the non-deformed heated bundle except that the inner duct wall bulges outward
gradually over the heated axial length to represent the geometry of an end-of-life duct (the deformed duct has a cosine shape in both the axial direction and transversely on every side, between each corner of the duct).

For all cases, a flat inlet velocity profile is assumed, which matches the benchmark specifications and the post-test simulation details report (APPENDIX H and APPENDIX E of the Mays and Jackson report [!citep](mays2017thermal)). The overall entry length of the bundle allows for the flow profile to be fully developed before the measurement region, so a flat inlet velocity profile is an appropriate condition. The fluid used is water.

The axial heat flux profile of the heated pins is a cosine with 1.4 peak/average, which is similar to the expected reactor flux and typical of heater rods. The axial heat flux profile is given by the following equation:

\begin{equation}
    p(z) = \frac{0.4\pi}{\pi - 2}  sin(\pi  \frac{z}{L}) + 1.4 -  \frac{0.4\pi}{\pi - 2},
\end{equation}

where $L$ is the length of the heated section and $z$ is the axial location from the beginning of the heated section.

TerraPower was responsible for the RANS CFD computations. The CD-Adapco commercial CFD suite Star-CCM+ was used to solve the RANS post-test simulations of both heated bundles.

## SCM duct deformation model

SCM models the effect of the deformation of the duct by adapting the geometric parameters of the perimetric subchannels according to a representative per subchannel deformation variable, which is called Displacement (D). This variable is calculated based on the centroid coordinates of each subchannel. Particularly, the relative lateral location of the subchannel centroid in relation to the center of each deformed duct side ($x_i$) and the height ($z$). These values are plugged into the equation that describes the deformation, which is presented in [bundle]. The result is the value of Displacement D for each subchannel at a specific axial height.

The geometric parameters affected by Displacement D are the wetted perimeter, the gap and the surface area. This consequently leads to a change in the hydraulic diameter, friction factor, and friction force, among others, which, in turn, have an effect on the resolved flow field via the conservation of momentum equations. The effect of the displacement value on the geometric parameters is graphically presented in [displacement].

!media subchannel/v&v/displacement.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=displacement
       caption=SCM duct deformation modeling.


The green line at the top of [displacement] represents an exaggerated version of the deformation. Based on the centroid location (black circles), the value of the Displacement variable $D$ is calculated. This calculation is performed per subchannel and the result of this calculation is used to perturb the other geometric parameters. A different treatment is used for the side and corner subchannels.

For the side subchannels, located in the perimeter of the hexagonal assembly, only the surface area and the gap between the pins and the duct are affected by the duct deformation. The wetted perimeter remains the same. The deformation is assumed constant along the side of the subchannel and a rectangular (blue-shaded) shaped area is added to the original surface area of the subchannel. This is an approximation since the actual deformation has the shape depicted in [bundle] and the green line in [displacement].

For the corner subchannel the surface area, gap, and wetted perimeter are affected. In this case a lambda (blue-shaded) shaped area is added to the original surface area subchannel. Again, this is an approximation necessitated by the fact that in the subchannel formulation all values are considered in a per subchannel basis. The deformation is assumed to be symmetric across the line connecting the center of the corner pin to the corner of the duct.

## Input files

The SCM deformation model is implemented by an IC: [FCTFdisplacementIC](/ics/FCTFdisplacementIC.md)

The deformed duct simulation is run by the following input files:

!listing /examples/areva_FCTF/FCTF_deformed.i language=cpp

!listing /examples/areva_FCTF/deformed_duct_pp.i language=cpp

## Results

Pressure measurements were taken from a pressure tap in the middle of Face B (see [DP-FaceB]) at various axial heights from the start of the heated section of the bundle.

!media subchannel/v&v/DP-FaceB.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=DP-FaceB
       caption=Deformed duct pressure measurements.

In all cases SCM over-predicts pressure drop in comparison to CFD and the experiment except for the half pitch ($0-\frac{1}{2}P$) in the non-deformed duct case. The reason for this over-prediction is that there are wire-wrap/half-pitch effects that are not captured by the friction factor model [!citep](chen2018upgraded) implemented in SCM. The linear model implemented in the subchannel scale, calculates an axial/surface averaged friction factor, that does not capture effects in the sub-pitch, sub-subchannel scale, associated with the wire-wrap and TPG mentioned above. Contrary, the CFD models the effect of the wire-wrap in a more refined way, hence it better matches with the experimental results in all axial lengths.


Experimental temperature measurements are extracted from locations on the outer surface of specific unheated tubes, and on duct Face E, at two different axial heights. The first plane, known as Plane B, is at an axial height 4.4167P above the start of the heated section. At plane B the wire-wrapping is at 5 o'clock and the thermocouples are at 4 and 6 o'clock ([planeB]).

!media subchannel/v&v/planeB.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=planeB
       caption=Measurement plane B.

Pink dots show locations where temperatures were sampled from. Unheated tubes where temperatures are taken from are labeled with numbers 1-22 and wall locations where temperatures are taken from are labeled as w1-w5.

A plot of the temperature difference between the local temperature and the planar mean temperature for Planes B, where the planar mean is determined from an average of the local temperatures at the specified measurement points, is shown in [TEMP-PlaneB]. It should be noted that the experimental measurements and the CFD calculations, refer to point-wise surface measurements at the selected unheated pin surfaces. On the other hand, the SCM calculations refer to subchannel surface averages. For a more accurate representation of the locality of the pin surface temperature measure by the thermocouples, the selected subchannels that correspond to the pin temperature values, are those that are closer to both the thermocouple locations. More specifically, the two thermocouple values per pin and the two CFD calculations per pin, are averaged to give one value per pin which is associated with the value of the nearest subchannel.

!media subchannel/v&v/TEMP-PlaneB.png
       style=width:60%;margin-bottom:2%;margin:auto;
       id=TEMP-PlaneB
       caption=Deformed duct temperature measurements on plane B.

The plot above presents the temperature gradients on the plane along with the SCM calculations of average subchannel temperature in relation to the local temperatures at the specified thermocouple locations. Most of the SCM results are within the range of the experimental measurements $\pm 2C$, which corresponds to the uncertainty of the individual thermocouple $\pm 1C$. Judging by the plot it can be inferred that the code over-predicts the diffusion effects since the SCM calculations are more concentrated towards the zero line, compared to the experimental measurements and the CFD calculations. At the same time the SCM results exhibit the same relative shape as the experimental measurements and CFD, meaning that the dominant trend of the physics are captured: higher temperatures are observed on the pins closer to the interior of the bundle, as this is where the bulk of the heated pins are located.

The four values calculated by SCM (# 1,2,3,4) that lie outside of the error bars and exhibit the most error, correspond to the inner most unheated pins which are in the higher temperature region of the bundle. The error in the rest of the pins is much smaller. It should be noted again that experiment and CFD capture point-wise quantities while SCM produces surface averaged quantities, so some difference is to be expected.

## Conclusion

In summary, this work has presented a subchannel methodology for modeling duct deformation in Liquid Metal Fast Breeder Reactors. This methodology has been validated against the non-deformed and deformed duct cases of the project Thermal Hydraulic Computational Fluid Dynamics Simulations and Experimental Investigation of Deformed Fuel Assemblies.