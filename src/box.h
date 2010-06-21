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
      map<my_int, ExactD2Node*> _c_nodemap; //nodemap for caching
      map<my_int, ExactD2Node*> _q_nodemap; //nodemap for querying
      set<ExactD2Node*> _nodeset;
      my_int _c_start;  //column address of start
      my_int _c_end;    //column address of end
      my_int _r_start;  //row address of start
      my_int _r_end;    //row address of end
      my_int _c_mid;    // column address in middle
      my_int _r_mid;    // row address in middle
      int _max;
      int _min;
      //string _position;
      map<string, int> _positionmap;
      void setEmptyPositionMap();
    
    public:
      Box(my_int c_start, my_int c_end, my_int r_start, my_int r_end);
      void addNode(ExactD2Node* n, DeetooNetwork& net);
      void clearNodes();
      void deleteNode(ExactD2Node* n);
      //returns position as string "lu", "lb", "ru", "rb"
      string getPosition(ExactD2Node* n);
      // returns start and end address (_c_address, _r_address)
      vector<my_int> getBoundary();
      // returns true if addr is in the box, false otherwise
      bool inBox(ExactD2Node* node);
      bool isSplittable();
      void update(my_int start, my_int end);
      void updateMaps();
      void clearPositionMap();
      bool equalTo(Box* box);
      int count() { return _nodeset.size(); };
      my_int positionToRandomAddress(string pos, Random& r);
      string getDiagonalPosition(string pos);
      my_int getJoinAddress(Random& r);
      //@param isColumn true if column, false if row.
      //return splitted boundary
      vector<my_int> getSplittedBoundary(bool isColumn);
      //pair<my_int, my_int> getEmptyPosition();
      my_int getMiddle(bool isCol);
      map<string,int> getPositionMap() { return _positionmap; };
      my_int colrowToAddr(my_int col, my_int row);
      pair<my_int, my_int> addrToColRow(my_int addr);
      my_int getMid(my_int start, my_int end);
      pair<my_int, my_int> getBroadcastRange(bool isCol);
      // returns (_c_start, _c_end) if col, (_r_start, _r_end) if row
      pair<my_int, my_int> getAddrOfElement(bool isCol) ;
      void printNodes();
      bool splitColumn();
  };
}
#endif
