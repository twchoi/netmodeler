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

#ifndef starsky__exactd2node_h
#define starsky__exactd2node_h

#include "addressednode.h"
//#include "box.h"
#include <set>
//#define INT64
#ifdef INT64
  typedef unsigned long long my_int;
#else
  typedef unsigned long my_int;
#endif
using namespace std;
namespace Starsky {

  class Box;
  class ExactD2Node : public AddressedNode {
    protected:
      my_int _c_address;  //cache address(column)
      my_int _q_address;  //query address(row)
      my_int _addr_c;     //column address which tells column position
      my_int _addr_r;     //row address which tells row position
      set<my_int> _cols;
      set<my_int> _rows;
      Box* _my_box;

    public:
      //Box *my_box;
      ExactD2Node(const my_int addr, set<string> itemSet);
      ExactD2Node(const my_int addr, set<string> itemSet, set<my_int> cols, set<my_int> rows);
      //~ExactD2Node();
      my_int getRowAddress();
      my_int getColAddress();
      void setBox(Box* box) { _my_box = box; };
      //void updatebox(Box* box) { _my_box = box; }
      Box* getBox() { return _my_box; };
      set<my_int> getCols();
      set<my_int> getRows();
      void updateCols(set<my_int> cols);
      void updateRows(set<my_int> rows);
      void addColRow(my_int addr, bool isCol);
  };
}
#endif






