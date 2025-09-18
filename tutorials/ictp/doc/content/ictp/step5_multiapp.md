# Step 5: MultiApps id=ictp_step5

!---

- Create bulk fluid problem, which is fluid around the pin cell with the pin cell removed
- Set a fixed heat flux on the inner fluid problem (to be transferred later from the pin cell surface)
- Set a fixed temperature on the outer boundary
- Run just this problem on its own
- Setup multiapp where the pin cell transfers heat flux (via average postprocessor) to the fluid
