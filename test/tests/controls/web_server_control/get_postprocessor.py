#!/usr/bin/env python3
from base_controller import *
import csv
import math

if __name__ == '__main__':
    num_steps = 2

    with open('gold/get_postprocessor_out.csv', mode ='r') as file:
        gold = []
        for row in list(csv.reader(file))[1:]:
            gold.append(float(row[1]))

    def expect_close(v1, v2):
        if not math.isclose(v1, v2):
            raise Exception(f'"{v1}" != "{v2}"')

    def run_control(control):
        control.wait('INITIAL')
        value = control.getPostprocessor('t')
        expect_close(value, gold[0])
        control.setContinue()

        for t in range(len(gold) - 1):
            control.wait('TIMESTEP_BEGIN')

            value = control.getPostprocessor('t')
            expect_close(value, gold[t])

            control.setContinue()

    base_controller('web_server', run_control)
