# backstroke
A tool for reversible computation in C++ based on incremental state saving.

## Purpose

* Backstroke is a tool for reversible computation and supports
  C++11. It is a command line tool for generating reversible C++
  source code and is distributed with a runtime library that can
  maintain all data necessary to support reversibility. The runtime
  library implements a so called Runtime State Store (RTSS) and is
  called rtss library. Backstroke implements a technique for
  incremental state saving (also called incremental check-pointing).

* Backstroke supports the simulator ROSS for parallel discrete event
  simulation (PDES) and can establish automatically the reversibility
  of ROSS models.


## Download

* The Backstroke git repository can be found at
  https://github.com/LLNL/backstroke


## Installation

1. Install ROSE.

  Backstroke requires ROSE to be installed. ROSE is available at
  www.rosecompiler.org. 

2. Clone the backstroke repository and install Backstroke.

  Run "make install" in the root directory of the Backstroke
  distribution. Backstroke can be uninstalled with 'make uninstall'.

  Note, Backstroke does not require to run configure because it
  obtains all configuration information from the ROSE installation by
  calling the ROSE tool rose-config. The tool backstroke and the rtss
  library are installed at the same location as ROSE.


## User Manual

* The Backstroke user manual is available as part of the Backstroke
  distribution at:
  https://github.com/LLNL/backstroke/blob/master/docs/user-manual.pdf
