/** 
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

#include "exactd2node.h"

using namespace Starsky;
using namespace std;

//#define INT64
#ifdef INT64
  typedef unsigned long long my_int;
  #define WMAX 18446744073709551615LL
  #define AMAX 4294967296LL
#else
  typedef unsigned long my_int;
  #define AMAX 65536L
  #define WMAX 4294967295L
#endif

ExactD2Node::ExactD2Node(const my_int addr, set<string> itemset) : AddressedNode(addr, itemset)
{
}
ExactD2Node::ExactD2Node(const my_int addr, set<string> itemSet, set<my_int> cols, set<my_int> rows) {
  _cols = cols;
  _rows = rows;
  _addr = addr;
  //_addr_c_ = addr % AMAX;
  //_addr_r = (addr - _addr_c) / AMAX; 
  ExactD2Node(addr, itemSet, cols, rows);
}
set<my_int> ExactD2Node::getCols() {
  return _cols;
}
set<my_int> ExactD2Node::getRows() {
  return _rows;
}
pair<my_int,my_int> ExactD2Node::getColRowAddress() {
  my_int row_addr = _addr % AMAX;
  my_int col_addr = (_addr - row_addr) / AMAX;
  return make_pair(col_addr, row_addr);
}

void ExactD2Node::updateCols(set<my_int> cols) {
  _cols = cols; 
}
void ExactD2Node::updateRows(set<my_int> rows) {
  _rows = rows; 
}
void ExactD2Node::addColRow(my_int addr, bool isCol) {
  set<my_int>::const_iterator it; 
  if (isCol) {
    it = _cols.find(addr);
    if (it != _cols.end() ) {
      _cols.insert(addr);
    }
  }
  else {
    it = _rows.find(addr);
    if (it != _rows.end() ) {
      _rows.insert(addr);
    }
  }
}
/*
void ExactD2Node::getMyBox(Box* box) {
  my_box = box;
}
*/

