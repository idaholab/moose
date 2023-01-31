# Cross Section Preparation

!---

# Two/Three Step Workflow

!media griffin_workflow.png

!---

# ISOXML Format

```xml
<ISOXML>
  <Multigroup_Cross_Section_Libraries Name="serpent_pbr_xs" NGroup="2" Description="RR XSs">
    <Multigroup_Cross_Section_Library ID="1" Ver="1.0" Generator="INL" TimeCreated="Wed Jan 18 14:59:57 2023" Description="ufuel">
      <Tabulation>tf tAl bu</Tabulation>
      <tf>350 400 450</tf>
      <tAl>350 400 450</tAl>
      <bu>0 5.1717 10.3434 15.5151 20.6868 25.8585 31.0302 36.2019 41.3736 46.5453 51</bu>
      <ReferenceGridIndex>1 1 1</ReferenceGridIndex>
      <AllReactions>Absorption Fission Scattering Transport Removal NeutronVelocity nuFission kappaFission FissionSpectrum DNFraction DNSpectrum DNPlambda</AllReactions>
      <TablewiseReactions/>
      <LibrarywiseReactions/>
      <Table gridIndex="1 1 1">
        <Isotope Name="pseudo" I="8">
          <Absorption>0.16442 2.78512</Absorption>
          <Fission>0.0679567 2.32609</Fission>
          <nuFission>0.168286 5.66683</nuFission>
          <kappaFission>13.76320249 470.4982243</kappaFission>
          <NeutronVelocity>22177276.28 419222.1752</NeutronVelocity>
          <Transport>0.57009 3.24648</Transport>
          <Removal>0.164116 2.78515</Removal>
          <FissionSpectrum>1 0</FissionSpectrum>
          <DNSpectrum index="g">1 0</DNSpectrum>
          <DNFraction>0.000219307 0.00102248 0.000627983 0.0013053 0.00218544 0.000632249 0.000563557 0.000155839</DNFraction>
          <DNPlambda>0.0124667 0.0282917 0.0425244 0.133042 0.292467 0.666488 1.63478 3.5546</DNPlambda>
          <Scattering>
            0.474556 0.000262005
            0.000194847 0.46209
          </Scattering>
        </Isotope>
      </Table>
      <Table gridIndex="1 1 2">
        <Isotope Name="pseudo" I="8">
        ...
        </Isotope>
      </Table>
      ...
    </Multigroup_Cross_Section_Library>
    <Multigroup_Cross_Section_Library ID="2" Ver="1.0" Generator="INL" TimeCreated="Wed Jan 18 14:59:57 2023" Description="uplate">
    ...
    </Multigroup_Cross_Section_Library>
    <Multigroup_Cross_Section_Library ID="3" Ver="1.0" Generator="INL" TimeCreated="Wed Jan 18 14:59:58 2023" Description="uwater">
    ...
    </Multigroup_Cross_Section_Library>
  </Multigroup_Cross_Section_Libraries>
</ISOXML>
```

!---

# Neutronics Material

- Neutronics materials define cross sections as material properties
- Cross section depend on temperatures, burnup, etc. $\rightarrow$ multiphysics simulation must evaluate cross sections for relevant conditions

### Interpolation Grid for ISOXML

!media grid_interpolation.png style=width:50%;margin-left:auto;margin-right:auto;display:block;background-color:white;

!--

# Coupled Feedback Materials

!row!
!col! width=45%
- `CoupledFeedbackNeutronicsMaterial` evaluates cross sections at multiphysics conditions given by MOOSE variables
- Variable names are provided using parameter `grid_variables`
- Variable names matched to parameter `grid_names`

  ```xml
      <Tabulation>tf tAl bu</Tabulation>
  ```

- `CoupledFeedbackNeutronicsMaterial` allows mixing from microscopic cross sections or providing macro using pseudo isotopes and `densities = ‘1’`
!col-end!

!col! width=10%
!!
!col-end!

!col! width=45%
!listing!
[Materials]
 [fuel]
   type = CoupledFeedbackNeutronicsMaterial
    block = 'fuel'
    material_id = 1
    plus = true
    library_file = XS_research_reactor.xml
    library_name = serpent_pbr_xs

    grid_names = 'tf tAl bu'
    grid_variables = 'tf tAl bu'
    isotopes = pseudo
    densities = 1.0

    is_meter = true
  []
  ...
[]
!listing-end!
!col-end!
!row-end!

!---

# Assignment of Cross Section Regions

!row!
!col! width=45%
### Option 1: Using single block restriction

```
[Materials]
  [fuel]
    type = CoupledFeedbackNeutronicsMaterial
    block = fuel
    material_id = 1
    ...
  []
  [clad]
    type = CoupledFeedbackNeutronicsMaterial
    block = clad
    material_id = 2
    ...
  []
  [water]
    type = CoupledFeedbackNeutronicsMaterial
    block = water
    material_id = 3
    ...
  []
[]
```

!col-end!

!col! width=10%
!!
!col-end!

!col! width=45%
### Option 2: Using multiple block restriction

```
[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = neutronics_mesh_in.e
  []
  [assign_material_id]
    type = SubdomainExtraElementIDGenerator
    input = fmg
    subdomains = 'fuel clad water'
    extra_element_id_names = 'material_id'
    extra_element_ids = '1 2 3'
  []
[]

[Materials]
  [all]
    type = CoupledFeedbackMatIDNeutronicsMaterial
    block = 'fuel clad water'
    ...
  []
[]
```
!col-end!
!row-end!
