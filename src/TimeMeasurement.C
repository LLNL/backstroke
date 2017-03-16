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

#include "TimeMeasurement.h"
#include <stdexcept>
#include <cmath>     
#include <iomanip>

TimeDuration::TimeDuration():_timeDuration(0.0) {
}

TimeDuration::TimeDuration(double timeDuration):_timeDuration(timeDuration) {
}

TimeDuration TimeDuration::operator+(const TimeDuration &other) {
  return TimeDuration(_timeDuration+other._timeDuration);
}

#pragma reversible map forward=original
TimeDuration& TimeDuration::operator+=(const TimeDuration& rhs){
      this->_timeDuration += rhs._timeDuration;
      return *this;
}

double TimeDuration::microSeconds() {
  return _timeDuration;
}

double TimeDuration::milliSeconds() {
  return _timeDuration*0.001;
}   

double TimeDuration::seconds() {
  return _timeDuration* 0.000001;
}

double TimeDuration::minutes() {
  return seconds()/60.0;
}

double TimeDuration::hours() {
  return minutes()/60.0;
}

std::string TimeDuration::longTimeString() {
  int ms=std::floor(milliSeconds());
  int s=std::floor(seconds());
  int m=std::floor(minutes());
  int h=std::floor(hours());
  std::stringstream ss;
  ss<<std::setfill('0')<<std::setw(2)<<h<<":"<<m<<":"<<s<<"."<<std::setw(3)<<ms;
  return ss.str();
}

#pragma reversible map forward=original
TimeMeasurement::TimeMeasurement():startTimeInMicroSeconds(0),
               endTimeInMicroSeconds(0),
               state(TIME_STOPPED)
{
    startCount.tv_sec = startCount.tv_usec = 0;
    endCount.tv_sec = endCount.tv_usec = 0;
}

#pragma reversible map forward=original
void TimeMeasurement::start() {
  if(state==TIME_RUNNING) {
    throw std::runtime_error("Internal error 1: TimeMeasurement in wrong state (RUNNING).");
  } else {
    state=TIME_RUNNING;
    gettimeofday(&startCount, 0);
  }
}

#pragma reversible map forward=original
void TimeMeasurement::stop() {
  if(state==TIME_STOPPED) {
    throw std::runtime_error("Internal error 2: TimeMeasurement in wrong state (STOPPED).");
  } else {
    gettimeofday(&endCount, NULL);
    state=TIME_STOPPED;
  }  
} 

#pragma reversible map forward=original
TimeDuration TimeMeasurement::getTimeDuration() {
  if(state==TIME_RUNNING) {
    throw std::runtime_error("Internal error 3: TimeMeasurement in wrong state (RUNNING).");
  } else {
    return TimeDuration((endCount.tv_sec-startCount.tv_sec)*1000000.0+(endCount.tv_usec-startCount.tv_usec));
  }
}
