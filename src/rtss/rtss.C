/*

Copyright (c) 2017, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Markus Schordan (email: schordan1@llnl.gov)
LLNL-CODE-718438
All rights reserved.

This file is part of Backstroke. For details, see
https://github.com/LLNL/backstroke. Please also see the LICENSE file
for our additional BSD notice.

Redistribution of Backstroke and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the disclaimer below.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the disclaimer (as noted below)
  in the documentation and/or other materials provided with the
  distribution.

* Neither the name of the LLNS/LLNL nor the names of its contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL
SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING

IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "backstroke/rtss.h"

#include <pthread.h>
#include <iostream>
#include <cassert>
#include <stdlib.h>
#include <pthread.h>

using namespace std;

 uintptr_t Backstroke::RunTimeStateStore::prog_stack_bottom=0;
 uintptr_t Backstroke::RunTimeStateStore::prog_stack_local=0;
 uintptr_t Backstroke::RunTimeStateStore::prog_stack_max=0;

// used as a single instance in the xpdes namespace
Backstroke::RunTimeStateStore* xpdes::internalRTSS=0;

// used as a single instance in maintaing the lp->rtss mapping.
Backstroke::RunTimeLpStateStore* xpdes::rtlpss=0;

// global flag to enable/disable recording of information
bool Backstroke::RunTimeStateStore::_dataRecording=false;


#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::init_stack_info() {
  pthread_t self = pthread_self();
  pthread_attr_t attr;
  void *stack;
  std::size_t stacksize;
  pthread_getattr_np(self, &attr);
  pthread_attr_getstack(&attr, &stack, &stacksize);
  uintptr_t stackmax=(uintptr_t)((long int)stack+(long int)stacksize);
  prog_stack_bottom=(uintptr_t)stack;
  prog_stack_max=stackmax;
  prog_stack_local=prog_stack_max;
}

void Backstroke::RunTimeStateStore::print_stack_info() {
  std::cout<<"STACKTOP     : "<<prog_stack_max<<std::endl;
  std::cout<<"STACKLOCALTOP: "<<prog_stack_local<<std::endl;
  std::cout<<"STACKEND     : "<<prog_stack_bottom<<std::endl;
}
