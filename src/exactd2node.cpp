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
  ExactD2Node(addr, itemSet, cols, rows);
}
set<my_int> ExactD2Node::getCols() {
  return _cols;
}
set<my_int> ExactD2Node::getRows() {
  return _rows;
}
my_int ExactD2Node::getColAddress() {
  return _addr_c;
}
my_int ExactD2Node::getRowAddress() {
  return _addr_r;
}

void ExactD2Node::updateCols(set<my_int> cols) {
  _cols = cols; 
}
void ExactD2Node::updateRows(set<my_int> rows) {
  _rows = rows; 
}
/*
void ExactD2Node::getMyBox(Box* box) {
  my_box = box;
}
*/

