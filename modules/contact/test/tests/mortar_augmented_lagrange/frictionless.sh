#!/bin/bash

# Penalty only
moose -i frictionless.i Problem/type=FEProblem Outputs/file_base=penalty_out

# Augmented Lagrange
moose -i frictionless.i Outputs/file_base=al_out

# Augmented lagrange with various levels of prediction

for scale in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
do
	moose -i frictionless.i Outputs/file_base=al_predict_${scale}_out UserObjects/weighted_gap_uo/augmented_lagrange_predictor_scale=$scale
done
