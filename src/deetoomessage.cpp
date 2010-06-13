/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2005  University of California
Copyright (C) 2005  P. Oscar Boykin <boykin@pobox.com>, University of Florida
Copyright (C) 2006  Tae Woong Choi  <twchoi@ufl.edu>, University of Florida

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

#include <deetoomessage.h>
#include <assert.h>

using namespace Starsky;
using namespace std;
//#define INT64
#ifdef INT64
  typedef unsigned long long my_int;
#else
  typedef unsigned long my_int;
#endif

//typedef unsigned long long my_int;

DeetooMessage::DeetooMessage(my_int r0, my_int r1, bool cache, Random& r_num, double p_fail) : _r0(r0), _r1(r1), _cache(cache), _r_num(r_num), _p_fail(p_fail)
{
  /*
  if (r0 > r1) {
	cerr << "starting point should be less than ending point" << endl
		<< "(" << _r0 << ", " << _r1 << ")" << endl;
  }
  */
  _mid_range = (my_int)( ( (double)(_r0)+(double)(_r1) )/(double)(2) );
  out_edge_count = 1;
  init_node = NULL;
  #ifdef INT64
    _dist_to_lower = 18446744073709551615LL;
  #else  
    _dist_to_lower = 4294967295L;
  #endif
  insert_fail = false;
  _pp_success = false;
}
bool DeetooMessage::inRange(AddressedNode* inode)
{
  bool betw = inode->isBetweenFromLeft(_r0, _r1, _cache);
  my_int nd_addr = inode->getAddress(_cache);
  if (nd_addr == _r0 || nd_addr == _r1) { return true; }
  else { return betw; }
}
/*
bool DeetooMessage::inRange(AddressedNode* inode)
{
    my_int nd_addr = inode->getAddress(_cache);
    //return ( ( nd_addr >= _r0) && ( nd_addr <= _r1) );
    return ( ( nd_addr > _r0) && ( nd_addr <= _r1) );
}	
*/

/**
 * bi-directional visit
 */
DeetooNetwork* DeetooMessage::visit(Node* n, Network& net)
{
  DeetooNetwork* d2n = dynamic_cast<DeetooNetwork*>( net.newNetwork() );
  AddressedNode* start = dynamic_cast<AddressedNode*> (n);      //start node for broadcasting 
  //d2n->add(start);
  visit(start, net, *d2n);
  return d2n;
}
/**
 * Uni-directional visit
 */
DeetooNetwork* DeetooMessage::visitD1(Node* n, Network& net)
{
  DeetooNetwork* d2n = dynamic_cast<DeetooNetwork*>( net.newNetwork() );
  AddressedNode* start = dynamic_cast<AddressedNode*> (n);      //start node for broadcasting 
  //d2n->add(start);
  visitD1(start, net, *d2n);
  return d2n;
}

void DeetooMessage::visit(AddressedNode* start, Network& net, DeetooNetwork& visited_net)
{
  //For the network reliability test, with failure prob. p, returns nothing.
  double p_edgefail = _r_num.getDouble01();
  if (p_edgefail < _p_fail) { 
      insert_fail = true;
      return; }
  
  //cout << "start node: " << start->getAddress(_cache) << ", range(" << _r0 << "," << _r1 << ")"<< endl;
  // If start node is not in the range(_r0, _r1), find the closest neighbor to lower bound in range.
  if ( (!inRange(start) ) )  //node is not in this range
  //if ( !(start->isBetweenFromLeft(_r0,_r1,_cache) ) )  //node is not in this range
  {
      //cout << "=============out of range" << endl;
      AddressedNode* next_node=NULL;
      my_int current_dist_to_lower = _dist_to_lower;
      auto_ptr<NodeIterator> ni(net.getNeighborIterator(start) );
      while (ni->moveNext()  )
      {
          AddressedNode* c_node = dynamic_cast<AddressedNode*> (ni->current() );
	  if ( inRange(c_node) ) {
	      init_node = c_node;
	      return visit(c_node, net, visited_net);
	  }
	  else {
	      my_int dist = c_node->getDistanceTo(_mid_range, _cache) ;
              if ( dist < _dist_to_lower )
	      {
                  next_node = c_node;
	          _dist_to_lower = dist;
	      }
	  }
      }	
      if (current_dist_to_lower == _dist_to_lower) {
	  return;
      }
      //We have the closest neighbor to lower, start over there
      out_edge_count++;http://www.gppgolf.com/gppg/content/viewCart.php
      return visit(next_node, net, visited_net);
  }
  if ( init_node == NULL ) { init_node = start; }
  visited_net.add(start);
  //We are in the range, get the neighbors.
  //divide range to upper and lower.
  //get upper neighbors and lower neighbors.
  //std::map will sort them according to their distances, lowest first.
  //will divide neighbors to upeer and lower groups wrt its address
  pair<map<my_int, AddressedNode*>, map<my_int, AddressedNode*> > cons = getConnectionInfo(start, _r0, _r1, net);
  std::map<my_int, AddressedNode*> left_con = cons.first;  
  std::map<my_int, AddressedNode*> right_con = cons.second;
  //cout << "~~~~~~~~~~~left cons of " << start->getAddress(_cache) << endl;
  //printCons(left_con);
  //cout << "~~~~~~~~~~~right cons of " << start->getAddress(_cache) << endl;
  //printCons(right_con);
  
  //if ( start->isBetweenFromLeft(_r0,_r1,_cache) ) {
    if (_neighbor_count > 0 ) { 
      visitInRange(start, _r0, _r1, left_con, 1, net, visited_net);
      visitInRange(start, _r0, _r1, right_con, 0, net, visited_net);
    }
  //}

  return;
}
void DeetooMessage::visitD1(AddressedNode* start, Network& net, DeetooNetwork& visited_net)
{
  //cout << "start: " << start->getAddress(_cache) << "rg_start: " << _r0 << "rg_end: " << _r1 <<endl;
  if ( init_node == NULL ) { init_node = start; }
  visited_net.add(start);

  std::map<my_int, AddressedNode*> neighbors;  
  auto_ptr<NodeIterator> ni(net.getNeighborIterator(start) );
  while(ni->moveNext() )
  {
    AddressedNode* current_node = dynamic_cast<AddressedNode*> (ni->current() );
    //check if current node is within the range
    if (inRange(current_node) ) {
      my_int dist = start->leftDistanceTo(current_node,_cache);
      neighbors[dist]=current_node; 
    }
  }
  //cout << "print connections of " << start->getAddress(_cache) << endl;
  //printCons(neighbors);

  int count = 0;
  my_int rg_start, rg_end;
  map<my_int,AddressedNode*>::const_iterator it;
  for (it=neighbors.begin(); it!=neighbors.end(); it++)
  {
    AddressedNode* cur_node = it->second;
    my_int cur_addr = cur_node->getAddress(_cache);
    rg_start = cur_addr;
    if (count == neighbors.size() -1) { // The last connection
      //cout << "last connection  " << _r1 << endl;
      rg_end = _r1;
    } 
    else {
      it++;
      AddressedNode* next_node = it->second;
      my_int next_addr = next_node->getAddress(_cache);
      rg_end = next_addr;
      it--;
    }
    //cout << "range: " << rg_start << ", " << rg_end << endl;
    if ( rg_start != rg_end) {
      //cout << "this node: " << start->getAddress(_cache) << "con: " << cur_addr << "range: (" << rg_start << "," << rg_end << ")" << endl;
      visited_net.add(Edge(cur_node,start) );
      //cout << "``````````````(" << cur_node->getAddress(_cache) << "," << start->getAddress(_cache) << ") edge is added" << endl;
      DeetooMessage m = DeetooMessage(rg_start, rg_end, _cache, _r_num, _p_fail);
      m.visitD1(it->second, net, visited_net);
    }
    count++;
  }
  return;  
}

/*
 * Find neighbor connections within the range.
 * return map of distance and node for left and right neighbors
 */
pair<map<my_int,AddressedNode*>,map<my_int,AddressedNode*> > DeetooMessage::getConnectionInfo(AddressedNode* t_node, my_int start, my_int end, Network& net) {
  pair<map<my_int,AddressedNode*>,map<my_int,AddressedNode*> > ret; //return value
  my_int t_addr = t_node->getAddress(_cache);
  map<my_int, AddressedNode*> left_con; //left neighbors
  map<my_int, AddressedNode*> right_con; //right neighbors
  _neighbor_count = 0;
  auto_ptr<NodeIterator> ni(net.getNeighborIterator(t_node) );
  while(ni->moveNext() )
  {
    AddressedNode* current_node = dynamic_cast<AddressedNode*> (ni->current() );
    //check if current node is within the range
    my_int left_dist = t_node->leftDistanceTo(current_node,_cache);
    my_int right_dist = current_node->rightDistanceTo(t_node,_cache);
    //cout << "in while dist: " << left_dist << ", current_node: " << current_node->getAddress(_cache) << endl;
    if (current_node->isBetweenFromLeft(t_addr, end, _cache) ) {
      left_con[left_dist] = current_node;
    }
    else if (current_node->isBetweenFromLeft(start,t_addr,_cache) ){
      right_con[right_dist] = current_node;
    }
    _neighbor_count++;
  }
  ret = make_pair(left_con, right_con);
  return ret;
}

/*
void DeetooMessage::visitInRangeOld(AddressedNode* this_node, my_int start, my_int end, map<my_int,AddressedNode*> cons, bool left, Network& net, DeetooNetwork& visited_net) {
  if (cons.size() != 0) { //make sure if connection map is not empty!
    my_int this_addr = this_node->getAddress(_cache);
    my_int last = this_addr;
    int count = cons.size();
    int i = 0;
    //the first element of cons is the nearest.
    map<my_int, AddressedNode*>::const_iterator it;
    for (it = cons.begin(); it != cons.end(); it++) {
      AddressedNode* next_node = it->second;
      visited_net.add(Edge(this_node,next_node) );
      my_int next_addr = next_node->getAddress(_cache);
      //cout << "this_node, next_addr: " << this_addr << " ," << next_addr << endl;
      my_int front = last;
      my_int back = next_addr;
      my_int rg_start, rg_end;
      if (i == count-1) {  // The last bit
	if (left) {
          // the left farthest neighbor
	  rg_start = front;
	  rg_end = end;
	}
	else { //the right farthest neighbor
          rg_start = start;
	  rg_end = front;
	}
      }
      else {
        if (left) { //left connections
	  rg_start = front;
	  rg_end = back;
	}
	else {
          rg_start = back;
	  rg_end = front;
	}
      }
      //cout << "range: " << rg_start << ", " << rg_end << endl;
      DeetooMessage m = DeetooMessage(rg_start, rg_end, _cache, _r_num, _p_fail);
      m.visit(next_node, net, visited_net);
      i++;
      last = next_addr;
    } 
  }
}
*/
void DeetooMessage::visitInRange(AddressedNode* this_node, my_int start, my_int end, map<my_int,AddressedNode*> cons, bool left, Network& net, DeetooNetwork& visited_net) {
  my_int last_addr;
  my_int this_addr = this_node->getAddress(_cache);
  if (left) {
    last_addr = _r1;
  }
  else {
    last_addr = _r0;
  }
  //AddressedNode* last = NULL;
  my_int rg_start, rg_end;
  map<my_int, AddressedNode*>::reverse_iterator it;
  for (it = cons.rbegin(); it != cons.rend(); it++) {
    AddressedNode* next = it->second;
    my_int next_addr = next->getAddress(_cache);
    
    if (next == (cons.begin())->second ) {
      if (left) {
        rg_start = this_addr;
	rg_end = last_addr;
      }
      else {
        rg_start = last_addr;
	rg_end = this_addr;
      }
    }
    else {
      if (left) {
        rg_start = next_addr;
	rg_end = last_addr;
      }
      else {
        rg_start = last_addr;
        rg_end = next_addr;
      }
    }
    if (rg_start != rg_end) {
      visited_net.add(Edge(this_node,next) );
      DeetooMessage m = DeetooMessage(rg_start, rg_end,  _cache, _r_num, _p_fail);
      m.visit(next, net, visited_net);
    }
    if (left) {
      last_addr = next_addr -1;
    }
    else {
      last_addr = next_addr +1;
    }
  }
}
void DeetooMessage::printCons(map<my_int, AddressedNode*> cons) {
  map<my_int, AddressedNode*>::const_iterator it;
  for(it = cons.begin(); it != cons.end(); it++) {
    cout << "(dist,addr): (" << it->first << "," << (it->second)->getAddress(_cache) << ")" << endl;
  }
}

