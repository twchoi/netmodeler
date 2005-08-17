/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2005  University of California
Copyright (C) 2005  P. Oscar Boykin <boykin@pobox.com>, University of Florida

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef starsky__random_h
#define starsky__random_h

#include <stdexcept>

namespace Starsky {

/**
 * an abstract class to subclass to get random numbers
 */
class Random {

  public:
  
    void setDoubleRange(double high=1.0, double low=0.0);
    virtual double getDouble();
    virtual double getDouble(double high, double low=0.0);
    virtual double getDouble01() = 0;

    void setIntRange(int high, int low=0);
    virtual int getInt();
    virtual int getInt(int high, int low=0);
    
    void setBoolTrueBias(double p);
    virtual bool getBool();
    virtual bool getBool(double p) {return (getDouble01() < p); }

  protected:
      int _imin, _imax;
      double _dmin, _dmax;
      double _true_bias;
};

}

#endif
