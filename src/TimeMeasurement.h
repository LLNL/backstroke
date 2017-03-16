#ifndef TIME_MEASUREMENT_H
#define TIME_MEASUREMENT_H

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

/* 

   The class TimeMeasurement performs a simple wall time measurement
   using the linux system function gettimeofday. The measurement can
   be started with 'start()' and stopped with 'stop()'. The start and
   stop operations must match exactly, otherwise the functions throw a
   std::runtime_error exception. The function getTimeDuration only
   succeeds if the time measurement has been stopped (with stop()) before or
   was never started, otherwise is throws a std::runtime_error
   exception. By default the time duration is 0. The measured time
   duration is reported as type TimeDuration.

   example: TimeMeasurement tm; 
            tm.start(); ... ; tm.stop(); 
            TimeDuration d=tm.getTimeDuration();

   The class TimeDuration stores time in micro seconds and provides
   conversion functions to return time in milli seconds, seconds,
   minutes, and hours (double value). The function longTimeString
   provides a human readable time format. The class also provides
   overloaded operators to perform arithmetic calculations based on
   the TimeDuration type. Therefore one can convert also only the
   final result of a time calculation (e.g. when printed).

   example: TimeDuration phase1=tm1.getTimeDuration();
            TimeDuration phase2=tm2.getTimeDuration();
            std::cout<<"Measured time: "<<(d1+d2).milliSeconds()<<" ms."<<std::endl;
            std::cout<<"Measured time: "<<(d1+d2).seconds()<<" s."<<std::endl;

 */

#include <sys/time.h>
#include <string>
#include <sstream>

class TimeDuration {
 public:
  TimeDuration();
  TimeDuration(double timeDuration);
  std::string longTimeString();
  double microSeconds();
  double milliSeconds();
  double seconds();
  double minutes();
  double hours();
  TimeDuration operator+(const TimeDuration &other);
  TimeDuration& operator+=(const TimeDuration& rhs);
 private:
  double _timeDuration;
};

enum TimeMeasurementState { TIME_RUNNING, TIME_STOPPED };

class TimeMeasurement {
 public:
  TimeMeasurement();
  virtual void start();
  virtual void stop();
  virtual TimeDuration getTimeDuration();
 protected:
  TimeMeasurementState state;
 private:
  double startTimeInMicroSeconds;
  double endTimeInMicroSeconds;
  timeval startCount;
  timeval endCount;
};

#endif
