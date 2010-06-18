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

Box::Box(my_int c_start, my_int c_end, my_int r_start, my_int r_end) {
  _c_start = c_start;
  _c_end = c_end;
  _r_start = r_start;
  _r_end = r_end;
  _c_mid = getMid(_c_start, _c_end);
  _r_mid = getMid(_r_start, _r_end);
  setEmptyPositionMap();
}
void Box::setEmptyPositionMap() {
  _positionmap["ru"] = 0;
  _positionmap["lu"] = 0;
  _positionmap["rb"] = 0;
  _positionmap["lb"] = 0;
}
void Box::addNode(ExactD2Node* n, DeetooNetwork& net) {
  my_int c_addr = n->getAddress(1);
  my_int q_addr = n->getAddress(0);
  /*
  map<my_int, ExactD2Node*>::const_iterator nit;
  for (nit = _nodemap.begin(); nit != _nodemap.end(); nit++) {
    if (!(net.getEdge(n,nit->second)) && !(net.getEdge(nit->second,n))) {
      net.add(Edge(n,nit->second) );
    }
  }
  map<my_int, ExactD2Node*>::iterator c_idx = _c_nodemap.find(c_addr);
  if (idx == _c_nodemap.end() ) {
    _c_nodemap[c_addr] = n;
    string pos = getPosition(n);
    //cout << "THIS POSITION?????????????   " << pos << endl;
    _positionmap[pos] = _positionmap[pos] + 1;
  }
  map<my_int, ExactD2Node*>::iterator q_idx = _q_nodemap.find(addr);
  if (idx == _q_nodemap.end() ) {
    _q_nodemap[addr] = n;
    //string pos = getPosition(n);
    //cout << "THIS POSITION?????????????   " << pos << endl;
    //_positionmap[pos] = _positionmap[pos] + 1;
  }
  */
  set<ExactD2Node*>::const_iterator idx = _nodeset.find(n);
  if (idx == _nodeset.end() ) {
    _nodeset.insert(n);
    string pos = getPosition(n);
    _positionmap[pos] = _positionmap[pos] + 1;
  }
  else {
    // this node is already in this box
  }

}
void Box::deleteNode(ExactD2Node* n) {
  set<ExactD2Node*>::const_iterator idx = _nodeset.find(n);
  if (idx != _nodeset.end() ) {
    _nodeset.erase(idx);
    string pos = getPosition(n);
    _positionmap[pos] = _positionmap[pos] -1;
  }
  else {
    cout << " the node is not in this box" << endl;
  }
}
void Box::resetNodes() {
  _nodeset.clear();
}
bool Box::inBox(ExactD2Node* node) {
  my_int addr = node->getAddress(1); // we only need caching address
                                       // because we can get both column and row elements from it
				       // column element is at caching side
				       // row element is at querying side
  //break address to column and row
  pair<my_int,my_int> cr_addrs = addrToColRow(addr);
  my_int addr_c =  cr_addrs.first; //this is node's position in caching node (column)
  my_int addr_r =  cr_addrs.second; // this is node's position in querying node (row)
  if (( (addr_c >= _c_start) && (addr_c < _c_end) ) && ((addr_r >= _r_start) && (addr_r < _r_end) ) ) {
    return true;
  }
  else {
    return false;
  }
}

string Box::getPosition(ExactD2Node* n) {
  string position;
  my_int addr = n->getAddress(1);
  pair<my_int, my_int> craddr = addrToColRow(addr);
  my_int my_col = craddr.first;
  my_int my_row = craddr.second;
  //cout << "c_start: " << _c_start << ", c_end: " << _c_end << ", c_mid: " << _c_mid << ", r_start: " << _r_start << ", r_end: " << _r_end << ", r_mid: " << _r_mid << endl;
  //cout << "myaddr: " << n->getAddress(1) << ", my_col: " << my_col << ", my_row: " << my_row << endl;
  if (my_col <= _c_mid) {
    if (my_row <= _r_mid) {
      position = "lu";   //"lu" is for left upper
    }
    else { // my_row < _r_mid
      position = "lb";   // "lb" is for left bottom
    }
  }
  else {  //my_col < _c_mid
    if (my_row <= _r_mid) {
      position = "ru";  // "ru" is for right upper
    }
    else {
      position = "rb";  // "rb" is for right bottom
    }
  }
  return position;
}
void Box::update(my_int start, my_int end) {
  _start = start;
  _end = end;
  pair<my_int, my_int> start_cr = addrToColRow(start);
  _c_start = start_cr.first;
  _r_start = start_cr.second;
  pair<my_int, my_int> end_cr = addrToColRow(end);
  _c_end = end_cr.first;
  _r_end = end_cr.second;
  _c_mid = getMid(_c_start, _c_end);
  _r_mid = getMid(_r_start, _r_end);
  updateMaps();
}

void Box::updateMaps() {
  clearPositionMap();
  //_nodemap.clear();
  map<my_int,ExactD2Node*>::const_iterator it;
  for (it = _nodemap.begin(); it != _nodemap.end(); it++) {
    my_int addr = (it->second)->getAddress(1);
    if (!inBox(addr) ) {
      _nodemap.erase(it->first);
    }
    else {
      //_nodemap[it->first] = it->second;
      string pos = getPosition(it->second);
      _positionmap[pos] = _positionmap[pos] + 1;
    }
  }

}

void Box::clearPositionMap() {
  _positionmap.clear();
  _positionmap["lu"] = 0;
  _positionmap["lb"] = 0;
  _positionmap["ru"] = 0;
  _positionmap["rb"] = 0;
}


bool Box::isSplittable() {
  if ( (_positionmap["lu"] >= 1 && _positionmap["rb"] >= 1)  || (_positionmap["lb"] >= 1 && _positionmap["ru"])) {
    return true;
  }
  else {
    return false;
  }
}
pair<my_int, my_int> Box::getBoundary() {
  return make_pair(this->_start, this->_end);
}

bool Box::equalTo(Box* box) {
  pair<my_int,my_int> box_addrs = box->getBoundary();
  if ( (_start == box_addrs.first) && (_end == box_addrs.second) ) {
    return true;
  }
  else {
    return false;
  }
  
}
my_int Box::positionToRandomAddress(string pos, Random& r) {
  my_int c_start, c_end, r_start, r_end;
  if (pos == "lu" || pos == "lb") {
    c_start = _c_start;
    c_end = _c_mid;
  }
  else {
    c_start = _c_mid;
    c_end = _c_end;
  }
  if (pos == "lu" || pos == "ru") {
    r_start = _r_start;
    r_end = _r_mid;
  }
  else {
    r_start = _r_mid;
    r_end = _r_end;
  }
  my_int col_addr = r.getInt(c_end, c_start);
  my_int row_addr = r.getInt(r_end, r_start);
  my_int address = colrowToAddr(col_addr, row_addr); 
  //cout << "pos: "<< pos << ", c_start: "<< c_start << ", c_end: " << c_end << ", r_start: " << r_start << ", r_end: " << r_end << endl;
  //cout << "col_addr: " << col_addr << ", row_addr: " << row_addr << ", addr: " << address << endl;
  return address;
}

my_int Box::getJoinAddress(Random& r) {
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
  if (count_zero == 3 || count_zero ==0) {
    // get diagonal position
    string dia_pos = getDiagonalPosition(max_pos);
    return positionToRandomAddress(dia_pos,r);
  } 
  else {
    return positionToRandomAddress(min_pos,r);
  }
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
vector<my_int> Box::getSplittedBoundary(bool isColumn) {
  cout << "~~~~~~~~~~~~~~~~~~~~~~ splitedboundary~~~~~~~~~~" << endl;
  my_int start1, start2, end1, end2;
  cout << "start: " << _start << ", end: " << _end << endl;
  vector<my_int> result;
  if (isColumn) {
    start1 = _start;
    end1 = colrowToAddr(_c_mid, _r_end);
    start2 = colrowToAddr((_c_mid+1), _r_start);
    //end1 = _c_mid * AMAX + _r_end; 
    //start2 = _c_mid * AMAX + _r_start;
    end2 = _end;  
    cout << "COL::::end1: " << end1 << " start2: " << start2 << endl;
  }
  else {
    start1 = _start;
    //end1 = _c_end * AMAX + _r_mid; 
    //start2 = _c_start * AMAX + _r_mid;
    //end1 = colrowToAddr(_c_end, _r_mid);
    //start2 = colrowToAddr((_c_start+1), _r_mid);
    end1 = colrowToAddr(_r_mid,_c_end);
    start2 = end1 + 1;
    end2 = _end;  
    cout << "ROW::::end1: " << end1 << " start2: " << start2 << endl;
  }  

  result.push_back(start1);
  result.push_back(end1);
  result.push_back(start2);
  result.push_back(end2);
  return result;
}
my_int Box::getMiddle(bool isCol) {
  if (isCol) { return _c_mid; }
  else { return _r_mid; }
}
my_int Box::colrowToAddr(my_int col, my_int row) {
  my_int addr = col * AMAX + row;
  return addr;
}

pair<my_int, my_int> Box::addrToColRow(my_int addr) {
  my_int row = addr % AMAX;
  my_int col = (addr - row) / AMAX;
  return make_pair(col, row);
}
my_int Box::getMid(my_int start, my_int end) {
  return (my_int)( (start + end) / 2.0);
}
pair<my_int, my_int> Box::getBroadcastRange(bool isCol) {
  my_int rg_start, rg_end;
  if ( isCol) {
    rg_start = _c_start * AMAX;
    rg_end = _c_end * AMAX + (AMAX -1);
  }
  else {
    rg_start = _r_start * AMAX;
    rg_end = _r_end * AMAX + (AMAX -1);
  }
  return make_pair(rg_start, rg_end); 
}
pair<my_int, my_int> Box::getAddrOfElement(bool isCol) {
  if (isCol) { return make_pair(_c_start, _c_end); }
  else { return make_pair(_r_start, _r_end); }
}
void Box::printNodes() {
  map<my_int, ExactD2Node*>::const_iterator it;
  cout << "nodes in this box are: " ;
  for (it = _nodemap.begin(); it != _nodemap.end(); it++) {
    cout << it->first << ", " ;
  }
  cout << endl;
}
bool Box::splitColumn() {
  if ((_c_end - _c_start) > (_r_end - _r_start) ) {
    return true;
  }
  else {
    return false;
  }
}
