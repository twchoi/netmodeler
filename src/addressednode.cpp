/** 
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2005  University of Florida
Copyright (C) 2005  P. Oscar Boykin <boykin@pobox.com>, University of Florida
Copyright (C) 2005  Tae Woong Choi <twchoi@ufl.edu>, University of Florida

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

#include "addressednode.h"

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

AddressedNode::AddressedNode()
{
  _c_address=0;
  _q_address=0;
}


AddressedNode::AddressedNode(const my_int addr, std::set<std::string> itemSet)
{
  _c_address = addr;
  addr_j = addr % AMAX;
  addr_i = (addr - addr_j) / AMAX;
  _q_address = addr_j*AMAX + addr_i;
  _itemSet = itemSet;
}

AddressedNode::AddressedNode(const my_int addr, std::map<string, pair<my_int, my_int> > objSet)
{
  _c_address = addr;
  addr_j = addr % AMAX;
  addr_i = (addr - addr_j) / AMAX;
  _q_address = addr_j*AMAX + addr_i;
  _objSet = objSet;
}

my_int AddressedNode::getAddress(bool cache)
{
    if (cache) { return _c_address; }
    else { return _q_address; }
}

my_int AddressedNode::getDistanceTo(my_int t_addr, bool cache)
{
  my_int this_addr;
  if (cache) { this_addr = _c_address; }
  else { this_addr = _q_address;}
  _small = std::min (this_addr, t_addr);
  _big = std::max (this_addr, t_addr);
  //ring distance
  _dist = std::min ((_big-_small), ( WMAX - _big + _small +1));
  return _dist;
}

bool AddressedNode::searchItem(std::string qItem)
{
  if (_itemSet.find(qItem) != _itemSet.end() ) {
    return true;
  }
  else {
    return false;
  }
}
bool AddressedNode::searchObject(string& qObj)
{
  
  bool ret=false;	
  if (_objSet.size() != 0) { 
    std::map<string, pair<my_int, my_int> >::iterator sit = _objSet.find(qObj);
    if (sit != _objSet.end() ) {
      ret = true;
    }
    else { ret = false; }
  }
  return ret;
}
void AddressedNode::insertItem(std::string& item)
{
    _itemSet.insert(item);
}

void AddressedNode::insertObject(string& item, my_int& a, my_int& b)
{
  _objSet[item] = make_pair(a, b);
}

void AddressedNode::deleteItem(std::string item)
{
    _itemSet.erase(item);
}
void AddressedNode::deleteObject(string obj)
{
  //erase() need iterator 
  std::map<string, pair<my_int, my_int> >::iterator it = _objSet.find(obj);
  _objSet.erase(it);
}
void AddressedNode::stabilize(int cq_size)
{
  //cout << "addr: " << _c_address << "\t" ;
  my_int new_start;
  my_int new_end;
  my_int mid = 0;
  map<string, pair<my_int, my_int> >::iterator obj_it;
  for (obj_it = _objSet.begin(); obj_it != _objSet.end(); obj_it++) {
    my_int old_start = obj_it->second.first;
    my_int old_end = obj_it->second.second;
    mid = (my_int)((old_start/2) + (old_end/2) );
    my_int range_size = getRangeSize(cq_size);
    new_start = mid - (range_size/2);
    new_end = mid + (range_size/2);
    //cout << "------cqsize------: " << cq_size << endl;
    /*
    cout << obj_it->first << "\t***********************************" << endl;
    cout << "oldstart: " << old_start << ", oldend: " << old_end << ", mid: " << mid << endl;
    cout << "start: " << new_start << ", end: " << new_end << endl;
    cout << "o_gap: " << (old_end-old_start) << ", n_gap: " << (new_end-new_start) << endl;
    */
    obj_it->second.first = new_start;
    obj_it->second.second = new_end;
    if( _c_address >= new_start && _c_address <= new_end) {
      //this object is withint the range. keep this object.
      //cout << "~~~~~~~~~ in the range~~~~~~~~~~" << endl;
    }
    else {
      _objSet.erase(obj_it);
      //cout << "===================out of range========" << endl;
    }
  }
}
my_int AddressedNode::getRangeSize(double cq_size) {
  my_int range0, range1;
  my_int range_size = 0;
  my_int start_cr = 10;
  double end_cr = (double)start_cr + cq_size-1;
  range0 = start_cr * AMAX;
  range1 = (my_int) ( (end_cr * AMAX) + AMAX -1 );
  range_size = range1 - range0;
  return range_size;
}
