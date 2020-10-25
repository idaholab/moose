# ActivateElementsCoupled

This user object uses the +coupled variable value+ as the metric to activate (add) an element by moving the element from an "inactive" subdomain to the "active" subdomain. It uses a coupled variable to decide whether to activate an element. The `coupled_var`, `activate_value` and the `activate_type` needs to be provided in the input to form the activation criterion. By default, the element is activated if the averaged value of the coupled variable in the element is `above` the `activate_value`. User can set `activate_type = 'below'` or `'equal'` to activate element when the averaged coupled variable value is below or equal to the `activate_value`.

!syntax parameters /UserObjects/ActivateElementsCoupled

!syntax inputs /UserObjects/ActivateElementsCoupled

!syntax children /UserObjects/ActivateElementsCoupled
