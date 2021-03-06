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
void Box::addNode(ExactD2Node* n) {
  set<ExactD2Node*>::const_iterator idx = _nodeset.find(n);
  if (idx == _nodeset.end() ) {
    //cout << "add this node: " << n->getAddress(1) << " in box: " << this<< endl;
    _nodeset.insert(n);
    string pos = getPosition(n);
    //cout << "position: " << pos << endl;
    _positionmap[pos] = _positionmap[pos] + 1;
  }
  else {
    // this node is already in this box
    //cout << "already in this box" << endl;
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
    //cout << " the node is not in this box" << endl;
  }
}
void Box::clearNodes() {
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
  if (( (addr_c >= _c_start) && (addr_c <= _c_end) ) && ((addr_r >= _r_start) && (addr_r <= _r_end) ) ) {
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
void Box::update(my_int c_start, my_int c_end, my_int r_start, my_int r_end) {
  _c_start = c_start;
  _c_end = c_end;
  _r_start = r_start;
  _r_end = r_end;
  _c_mid = getMid(_c_start, _c_end);
  _r_mid = getMid(_r_start, _r_end);
  updateMaps();
}

void Box::updateMaps() {
  clearPositionMap();
  set<ExactD2Node*>::const_iterator it;
  for (it = _nodeset.begin(); it != _nodeset.end(); it++) {
    if (!inBox(*it) ) {
      _nodeset.erase(it);
    }
    else {
      string pos = getPosition(*it);
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
vector<my_int> Box::getBoundary() {
  vector<my_int> result;
  result.clear();
  result.push_back(_c_start);
  result.push_back(_c_end);
  result.push_back(_r_start);
  result.push_back(_r_end);
  return result;
}

bool Box::equalTo(Box* box) {
  vector<my_int> boundary = box->getBoundary();
  if ( (_c_start == boundary[0]) && (_c_end == boundary[1]) && (_r_start == boundary[2]) && (_r_end == boundary[3]) ) {
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
  /*
  cout << "this box? " << this << endl;
  cout << "in getJoinAddress" << endl;
  cout << "pos map size: " << _positionmap.size() << endl;
  cout << "pos map: " << _positionmap["ru"] << "," << _positionmap["lu"] << "," << _positionmap["rb"] << ","<< _positionmap["lb"] << endl; 
  */
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
  //cout << "end of position map iteration" << endl;
  if (count_zero == 3 || count_zero ==0) {
    // get diagonal position
    //cout << "get diagonal position" << endl;
    string dia_pos = getDiagonalPosition(max_pos);
    return positionToRandomAddress(dia_pos,r);
  } 
  else {
    //cout << "get address in min postion: "<< endl;
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
  my_int c_start1, c_end1, c_start2, c_end2, r_start1, r_end1, r_start2, r_end2;
  vector<my_int> result;
  if (isColumn) {
    c_start1 = _c_start;
    c_end1 = _c_mid;
    r_start1 = _r_start;
    r_end1 = _r_end;

    c_start2 = _c_mid + 1;
    c_end2 = _c_end;
    r_start2 = _r_start;
    r_end2 = _r_end;
    //cout << "COL::::end1: " << end1 << " start2: " << start2 << endl;
  }
  else {
    c_start1 = _c_start;
    c_end1 = _c_end;
    r_start1 = _r_start;
    r_end1 = _r_mid;

    c_start2 = _c_start;
    c_end2 = _c_end;
    r_start2 = _r_mid + 1;
    r_end2 = _r_end;
    //cout << "COL::::end1: " << end1 << " start2: " << start2 << endl;
  }  

  result.push_back(c_start1);
  result.push_back(c_end1);
  result.push_back(r_start1);
  result.push_back(r_end1);
  result.push_back(c_start2);
  result.push_back(c_end2);
  result.push_back(r_start2);
  result.push_back(r_end2);
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
    rg_start = colrowToAddr(_c_start, 0);
    rg_end = colrowToAddr(_c_end, (AMAX-1) );
  }
  else {
    //For row split, we need to use addresses in querying space.
    //In querying space, row address should be replaced with column address,
    //and vice versa.
    //Note that query_address = row_address * \sqrt(N) + column_address,
    //while cache_address = column_address * \sqrt(N) + row_address.
    //Thus, address for range also should be converted to the addresses
    //in Querying space.
    rg_start = colrowToAddr(_r_start, 0);
    rg_end = colrowToAddr(_r_end, (AMAX-1) );
  }
  return make_pair(rg_start, rg_end); 
}
pair<my_int, my_int> Box::getAddrOfElement(bool isCol) {
  if (isCol) { return make_pair(_c_start, _c_end); }
  else { return make_pair(_r_start, _r_end); }
}
void Box::printNodes() {
  set<ExactD2Node*>::const_iterator it;
  cout << "nodes in this box are: " ;
  for (it = _nodeset.begin(); it != _nodeset.end(); it++) {
    cout << (*it)->getAddress(true) << ", " ;
  }
  cout << endl;
}
bool Box::splitColumn() {
  if ((_c_end - _c_start) >= (_r_end - _r_start) ) {
    return true;
  }
  else {
    return false;
  }
}
Box* Box::splitBox(bool isCol,bool inBox) {
  vector<my_int> new_bound = getSplittedBoundary(isCol);
  my_int c_start1 = new_bound[0];
  my_int c_end1 = new_bound[1];
  my_int r_start1= new_bound[2];
  my_int r_end1= new_bound[3];

  my_int c_start2 = new_bound[4];
  my_int c_end2 = new_bound[5];
  my_int r_start2= new_bound[6];
  my_int r_end2= new_bound[7];

  Box* box0 = new Box(c_start1, c_end1, r_start1, r_end1);
  Box* box1 = new Box(c_start2, c_end2, r_start2, r_end2);
  /*
  cout << "********************* in splitBox() ********************* " << endl;
  cout << "this_box: " << this << ", range: " << _c_start  << ":" << _c_end << ":" << _r_start << ":" << _r_end << endl;

  cout << "box0: " << box0 << ", range: " << c_start1 << ":" << c_end1 << ":" << r_start1 << ":" << r_end1 << endl;
  cout << "box1: " << box1 << ", range: " << c_start2 << ":" << c_end2 << ":" << r_start2 << ":" << r_end2 << endl;
  */
  set<ExactD2Node*>::const_iterator nit;
  for(nit= _nodeset.begin(); nit != _nodeset.end(); nit++) {
    ExactD2Node* this_node = *nit;
    /*
    my_int addr = this_node->getAddress(true);
    pair<my_int, my_int> colrow = addrToColRow(addr);
    cout << "this node's col:row " << colrow.first << ":" << colrow.second << endl;
    */
    if (box0->inBox(this_node) ) {
      box0->addNode(this_node);
      this_node->setBox(box0);
    }
    else if (box1->inBox(this_node) ) {
      box1->addNode(this_node);
      this_node->setBox(box1);
    }
    else {
      //cout << "~~~~~~~~~~~~~~~~ in no box" << endl;
    }
    Box* box = this_node->getBox();
    if (this->equalTo(box)) {
      cout << "WARNING!!!!!!!!!!! box is not splitted" << endl;
    }
    //cout << "afer split: node: " << this_node->getAddress(isCol) << endl;
  }
  //cout << "before split: box's size: " << this->count() << endl;
  _nodeset.clear();
  /*
  cout << "box: " << this << " is obsolete" << endl;
  cout << "bound: " << _c_start << ":" << _c_end << ":" << _r_start << ":" << _r_end << endl;
  cout << "box's size " << this->count() << endl;
  vector<my_int> bound0 = box0->getBoundary();
  vector<my_int> bound1 = box1->getBoundary();
  cout << "box splitted: box0: " << box0 << ", count: " << box0->count() << endl;
  cout << "bound: " << bound0[0] << ":" << bound0[1] << ":" << bound0[2] << ":" << bound0[3] << endl;
  cout << "box splitted: box1: " << box1 << ", count: " << box1->count() << endl;
  cout << "bound: " << bound1[0] << ":" << bound1[1] << ":" << bound1[2] << ":" << bound1[3] << endl;
  */
  if (inBox) {
    if (box0->count() >= box1->count() ) {
      return box1;
    }
    else {
      return box0;
    }
  }
}
