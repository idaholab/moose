# FormLoss1PhaseBase

This is a base class for prescribing a form loss over a 1-phase flow channel.

The form loss is created by adding a [ADOneD3EqnMomentumFormLoss.md] kernel. This
creates a momentum loss term in the equation for $\rho u A$, that is applied over the full length of
the [FlowChannel1Phase.md] component.
