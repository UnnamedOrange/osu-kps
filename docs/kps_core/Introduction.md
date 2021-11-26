# kps_core/Introduction

kps_core is a module that calculates KPS together with related data. It provides interfaces of different input sources and outputs the result statically or dynamically. The performance and low coupling are in much consideration.

## Inputs and Outputs

Abstractly, the inputs and output are:

- Output:

  - Light data with a short loading time, such as total KPS in hard algorithm.
  - Heavy data with a short loading time, such as KPS of all individual keys in hard algorithm.
  - Heavy data with a long loading time, such as KPS in the recent 5 minutes in hard algorithm.

  All outputs are real-time because osu-kps is a dynamic analysis program.
  
- Input:

  - Real-time input data, such as keystrokes from a keyboard and keystrokes read from memory.
  - Real-time reference data, such as standard keystrokes read from memory.
  - Non-real-time input data, such as keystrokes read from a record.
  - Non-real-time reference data, such as standard keystrokes loaded from a beat-map.

At one time, there can be only one input source and one reference source. However, users can create as many output instances as they want, and multiplexing of each type of output should be considered, especially heavy data with a long loading time.
