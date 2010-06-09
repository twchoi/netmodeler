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

#include "box.h"

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

Box::Box(my_int start, my_int end) {
  _c_addr = start;
  _r_addr = end;
  _c_start = start % AMAX;
  _c_end = end % AMAX;
  _c_mid = (int)(_c_start + _c_end) /2;
  _r_start = (start - _c_start) / AMAX;
  _r_end = (end - _c_end) / AMAX;
  _r_mid = (int)(_r_start + _r_end) /2;
  setEmptyPositionMap();
}
void Box::setEmptyPositionMap() {
  _positionmap["ru"] = 0;
  _positionmap["lu"] = 0;
  _positionmap["rb"] = 0;
  _positionmap["lb"] = 0;
}
void Box::addNode(ExactD2Node* n, DeetooNetwork& net) {
  my_int addr = n->getAddress(1);
  map<my_int, ExactD2Node*>::const_iterator nit;
  for (nit = _nodemap.begin(); nit != _nodemap.end(); nit++) {
    if (!(net.getEdge(n,nit->second)) && !(net.getEdge(nit->second,n))) {
      net.add(Edge(n,nit->second) );
    }
  }
  _nodemap[addr] = n;
  string pos = getPosition(n);
  _positionmap[pos] = _positionmap[pos] + 1;
}
void Box::deleteNode(ExactD2Node* n) {
  my_int addr = n->getAddress(1);
  map<my_int, ExactD2Node*>::iterator idx = _nodemap.find(addr);
  _nodemap.erase(idx);
  string pos = getPosition(n);
  _positionmap[pos] = _positionmap[pos] -1;
}
void Box::clearNodes() {
  _nodemap.clear();
}
bool Box::inBox(my_int addr) {
  my_int addr_c = addr % AMAX;
  my_int addr_r = (addr - addr_c) / AMAX;
  if (( (addr_c >= _c_start) && (addr_c < _c_end) ) && ((addr_r <= _r_start) && (addr_r < _r_end) ) ) {
    return true;
  }
  else {
    return false;
  }
}

string Box::getPosition(ExactD2Node* n) {
  string position;
  my_int my_col = n->getColAddress();
  my_int my_row = n->getRowAddress();
  my_int c_half = (my_int)((_c_start + _c_end) /2.0);
  my_int r_half = (my_int)((_r_start + _r_end) /2.0);
  if (my_col >= c_half) {
    if (my_row >= r_half) {
      position = "lu";   //"lu" is for left upper
    }
    else { // my_row < r_half
      position = "lb";   // "lb" is for left bottom
    }
  }
  else {  //my_col < c_half
    if (my_row >= r_half) {
      position = "ru";  // "ru" is for right upper
    }
    else {
      position = "rb";  // "rb" is for right bottom
    }
  }
  return position;
}
void Box::update(my_int start, my_int end) {
  _c_addr = start;
  _r_addr = end;
  _c_start = start % AMAX;
  _c_end = end % AMAX;
  _c_mid = (int)(_c_start + _c_end) /2;
  _r_start = (start - _c_start) / AMAX;
  _r_end = (end - _c_end) / AMAX;
  _r_mid = (int)(_r_start + _r_end) /2;
}

bool Box::isSplittable() {
  //vector<int> positions;
  //positions.assign(4);
  int lu=0, lb=0, ru=0, rb=0;
  map<my_int,ExactD2Node*>::iterator iter;
  for (iter = _nodemap.begin(); iter != _nodemap.end(); iter++) {
    string this_pos = getPosition(iter->second);
    if (this_pos == "lu") {
      lu++;
    }
    else if (this_pos == "lb") {
      lb++;
    }
    else if (this_pos == "ru") {
      ru++;
    }
    else if (this_pos == "rb") {
      rb++;
    }
    else { //there is no possible case except the above positions
	   // Do nothing
    }
  }
  /*
  positions.pushback(lu); //0
  positions.pushback(lb); //1
  positions.pushback(ru); //2
  positions.pushback(rb); //3

  if (positions[0] >= 1 && positions[3] >= 1) || (positions[1] >= 1 && positions[2]) {
  */
  if ((lu >= 1 && rb >= 1) || (lb >= 1 && ru >= 1) ) {
    return true;
  }
  else {
    return false;
  }
}
pair<my_int, my_int> Box::getBoundary() {
  return make_pair(this->_c_addr, this->_r_addr);
}

bool Box::equalTo(Box* box) {
  pair<my_int,my_int> box_addrs = box->getBoundary();
  if ( (_c_addr == box_addrs.first) && (_r_addr == box_addrs.second) ) {
    return true;
  }
  else {
    return false;
  }
  
}
pair<my_int, my_int> Box::positionToRange(string pos) {
  my_int start, end;
  if (pos == "lu") {
    start = _c_start * AMAX + _r_start;
    end = _c_mid * AMAX + _r_mid;
  }
  else if (pos == "lb") {
    start = _c_start * AMAX + _r_mid;
    end = _c_mid * AMAX + _r_end;
  }
  else if (pos == "ru") {
    start = _c_mid * AMAX + _r_start;
    end = _c_end * AMAX + _r_mid;
  }
  else {
    //pos = "rb"
    start = _c_mid * AMAX + _r_mid;
    end = _c_end * AMAX + _r_end;
  }
  return make_pair(start, end);
}
string Box::getDiagonalPosition(string pos) {
  if (pos == "lu") {
    return "rb";	 
  } 
  else if (pos == "rb") {
    return "lu";
  }
  else if (pos == "ru") {
    return "lb";
  }
  else { //pos == "lb"
    return "ru";
  }
}
pair<my_int, my_int> Box::getJoinPosition() {
  string min_pos;
  string max_pos;
  int min = 10;
  int max = 0;
  int count_zero = 0;
  map<string,int>::const_iterator it;
  for(it = _positionmap.begin(); it != _positionmap.end(); it++) {
    int count = it->second;
    if (count == 0) {
      count_zero++;
    }
    if (count < min) {
      min = count;
      min_pos = it->first;
    }
    else if (count > max) {
      max = count;
      max_pos = it->first;
    }
  }
  if (count_zero == 3) {
    return positionToRange(max_pos);
  } 
  else {
    return positionToRange(min_pos);
  }

}
vector<my_int> Box::getSplittedBoundary(bool isColumn) {
  my_int start1, start2, end1, end2;
  vector<my_int> result;
  if (isColumn) {
    start1 = _c_addr;
    end1 = _c_mid * AMAX + _r_end; 
    start2 = _c_mid * AMAX + _r_start;
    end2 = _r_addr;  
  }
  else {
    start1 = _c_addr;
    end1 = _c_end * AMAX + _r_mid; 
    start2 = _c_start * AMAX + _r_mid;
    end2 = _r_addr;  
  }  

  result.push_back(start1);
  result.push_back(end1);
  result.push_back(start2);
  result.push_back(end2);
  return result;
}
