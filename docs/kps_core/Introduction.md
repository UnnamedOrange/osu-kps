# kps_core/Introduction

kps_core is a module that calculates KPS together with related data. It provides interfaces of different input sources and outputs the result statically or dynamically. The performance and low coupling are in much consideration.

## Inputs and Outputs

Abstractly, the inputs and output are:

- Output:

  - Light data with a short loading time, such as total KPS in hard algorithm.
  - Heavy data with a short loading time, such as KPS of all individual keys in hard algorithm.
  - Heavy data with a long loading time, such as KPS in the recent 5 minutes in hard algorithm.

  All outputs are real-time because osu-kps is a dynamic analysis program.

