/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2010  University of Florida
Copyright (C) 2010  Tae Woong Choi <twchoi@ufl.edu>, University of Florida

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

#ifndef starsky__box_H
#define starsky__box_H

//#include "netmodeler.h"
#include "exactd2node.h"
#include "deetoonetwork.h"
#include <map>
#include <memory>
//#define INT64
#ifdef INT64
  typedef unsigned long long my_int;
#else
  typedef unsigned long my_int;
#endif


using namespace std;
namespace Starsky {
  
  class Box  {
    protected:
      map<my_int, ExactD2Node*> _nodemap;
      my_int _c_addr;
      my_int _r_addr;
      my_int _c_start;
      my_int _c_end;
      my_int _r_start;
      my_int _r_end;
      int _max;
      int _min;
      //map<string, pair<my_int, my_int> _positionmap;
    
    public:
      Box(my_int start, my_int end);
      void addNode(ExactD2Node* n, DeetooNetwork& net);
      void clearNodes();
      void deleteNode(ExactD2Node* n);
      string getPosition(ExactD2Node* n);
      pair<my_int, my_int> getBoundary();
      bool inBox(my_int addr);
      bool isSplittable();
      void update(my_int c_start, my_int c_end, my_int r_start, my_int r_end);
      bool equalTo(Box& box);
      int count() { return _nodemap.size(); };
      //pair<my_int, my_int> getEmptyPosition();
  };
}
#endif
