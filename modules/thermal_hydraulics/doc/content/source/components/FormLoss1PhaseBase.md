# FormLoss1PhaseBase

This is a bass class for prescribing a form loss over a 1-phase flow channel.

The form loss is created by adding a [ADOneD3EqnMomentumFormLoss.md] kernel. This
creates a momentum loss term in the equation for $\rho u A$, that is applied over all
the [FlowChannel1Phase.md] component.
