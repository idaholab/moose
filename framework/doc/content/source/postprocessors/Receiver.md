# Receiver

!syntax description /Postprocessors/Receiver

## Overview

The Receiver Postprocessor is useful for reporting scalar values created in other parts of the system
such as in a [MultiApp](syntax/MultiApps/index.md), and moved to the Receiver using a
[MultiAppPostprocessorTransfer.md] for example.
It does +not+ compute its own value. Note that the user may set a default
value with the "default" parameter.

## Example Input File Syntax

In this example, the value of the Receiver 'pp' in the subapp 'quad' is being populated by
the value of a variable 'parent_aux' in the main appplication.

!listing test/tests/transfers/multiapp_variable_value_sample_transfer/pp_sub.i block=Postprocessors caption='Snippet from the subapp showing the Receiver'

!listing test/tests/transfers/multiapp_variable_value_sample_transfer/pp_parent.i block=Transfers caption='Snippet from the main app, populating the Receiver with a transfer of the variable value at different points'

!syntax parameters /Postprocessors/Receiver

!syntax inputs /Postprocessors/Receiver

!syntax children /Postprocessors/Receiver
