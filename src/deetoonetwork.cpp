/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2005  University of California
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

#include "deetoonetwork.h"
#include <cmath>

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

DeetooNetwork::DeetooNetwork(Random& r) : Network(), _r_short(r) {}
DeetooNetwork::DeetooNetwork(int nodes, Random& r) : _r_short(r) {}

void DeetooNetwork::formRing(const std::map<my_int, AddressedNode*>& nodeMap) {
  AddressedNode* first = nodeMap.begin()->second;
  AddressedNode *tmp, *last = first;
  add(first);
  std::map<my_int, AddressedNode*>::const_iterator itNodeMap;
  for (itNodeMap = nodeMap.begin(); itNodeMap != nodeMap.end(); itNodeMap++) {
    if (itNodeMap == nodeMap.begin() ) {
      //do nothing. it is already added before for loop.
    }
    else {
      tmp = itNodeMap->second;
      if (!this->has(Edge(tmp, last) ) ) {
        add(Edge(tmp, last) );
      }
      last = tmp;
    }
  }
  add(Edge(last,first) );
}

void DeetooNetwork::makeShortcutConnection(const std::map<my_int, AddressedNode*>& nd_map, bool cache) {
  auto_ptr<NodeIterator> ni( getNodeIterator() );
  while(ni->moveNext() ) {
    AddressedNode* nodei = dynamic_cast<AddressedNode*> (ni->current() );
    my_int shortcut_address = nodei->getAddress(cache);
    while (shortcut_address == nodei->getAddress(cache) ) {
        double x = _r_short.getDouble01();
	//my_int net_size = guessNetSize(nodei, cache);
	//my_int net_size = guessNetSizeLog(nodei, cache);
	int net_size = getNodeSize();
        my_int k = (my_int) (pow(10,(log10(WMAX)-(1-x)*log10(net_size ) ) ) );
        my_int shortcut_target_addr = (nodei->getAddress(cache) + k) % (WMAX );
	AddressedNode* nodej = findShortcutNode(nd_map, shortcut_target_addr);
	shortcut_address = nodej->getAddress(cache);
        if ((nodei->getAddress(cache) != nodej->getAddress(cache)) && !(getEdge(nodei, nodej)) && !(getEdge(nodej, nodei))){
          add( Edge(nodei,nodej) );
	}
    }
  }
}
AddressedNode* DeetooNetwork::returnShortcutNode(AddressedNode* node, const std::map<my_int, AddressedNode*>& nd_map, bool cache) {
  AddressedNode* nodej;
    my_int shortcut_address = node->getAddress(cache);
    while (shortcut_address == node->getAddress(cache) ) {
        double x = _r_short.getDouble01();
	//my_int net_size = guessNetSizeLog(node, cache);
	my_int net_size = node_map.size();
	//int net_size = getNodeSize();
        my_int k = (my_int) (pow(10,(log10(WMAX)-(1-x)*log10(net_size ) ) ) );
        my_int shortcut_target_addr = (node->getAddress(cache) + k) % (WMAX );
	nodej = findShortcutNode(nd_map, shortcut_target_addr);   
	//cout << "addr: " << node->getAddress(cache) << ", " << "sc_addr: " << nodej->getAddress(cache) << endl;
	shortcut_address = nodej->getAddress(cache);
    }
  
  return nodej;
}

void DeetooNetwork::create(int n) {
    //Let's make n different nodes!!
    node_map.clear();
    // 1. Random network
    while(node_map.size() < n) 
    {
	my_int r_addr = (my_int)(_r_short.getDouble01() * (WMAX) );
	std::set<std::string> items;
	items.clear();
	if (node_map.find(r_addr) == node_map.end() && r_addr != 0){
            AddressedNode* anode = new AddressedNode(r_addr, items);
	    node_map[r_addr] = anode;
	    add(anode);
	}
    }
    // 2. uniform network
    /**
    while(node_map.size() < n)
    //for (my_int u_addr = 0; u_addr <= WMAX; u_addr += step)
    {
      std::set<std::string> items;
      items.clear();
      //cout << "u_addr: " << u_addr << endl;
      AddressedNode* anode = new AddressedNode(u_addr, items);
      node_map[u_addr] = anode;
      add(anode);
      u_addr += step;
    }
    */
    /**
    // 3. diagonal network
    double step = (double)(AMAX/(double)n);
    //cout << "step: " << step << endl;
    my_int t_addr = 0;
    while(node_map.size() < n)
    {
      std::set<std::string> items;
      items.clear();
      my_int addr = t_addr << 16 | t_addr;
      //cout << "t_addr: " << t_addr << "\taddr: " << addr << endl;
      //cout << "node map's size, addr: " << node_map.size() << "\t" << addr << endl;
      AddressedNode* anode = new AddressedNode(addr,items);
      //cout << "----------------------------------------------------" << endl
      //       << "\t cache addr: \t" << addr << "\t query addr: \t" << anode->getAddress(0) << endl;
      
      node_map[addr] = anode;
      add(anode);
      t_addr += step;
    }
    */

    //Form ring.
    formRing(node_map);
    //Establishing shortcut connections:
    makeShortcutConnection(node_map, true);
    //printNetInfo(true);
}
void DeetooNetwork::createNullNet() {
  node_map.clear();
}
void DeetooNetwork::createQueryNet(const std::map<my_int, AddressedNode*>& nd_map)
{
    //using addresses in query space, make node map for query.
    //query_nm.clear();
    node_map.clear();
    my_int query_addr=0;
    AddressedNode* q_node;
    std::map<my_int, AddressedNode*>::const_iterator itNd_map;
    for (itNd_map = nd_map.begin(); itNd_map != nd_map.end(); itNd_map++)
    {
	query_addr = (itNd_map->second)->getAddress(false);
	
	//cout << "----------------------------------------------------" << endl
        //	<< "\t query addr: \t" << query_addr << endl
	//	<< "\t cache addr: \t" << (itNd_map->second)->getAddress(true) << endl;
		
	q_node = dynamic_cast<AddressedNode*> (itNd_map->second);
	add(q_node);
	node_map[query_addr] = q_node;
    }
    
    //form ring.
    formRing(node_map);
    //make shortcut connection.
    makeShortcutConnection(node_map,false);
    //printNetInfo(false);
    
}
/**
AddressedNode* DeetooNetwork::getNodeFromAddress(const int addr) const {
	map<int, AddressedNode*>::const_iterator i = _node_map.find(addr);
	if (i != _node_map.end()) {
		return i->second;
	}
	return 0;
}*/

// Find the shortcut node
// we have target address, let's find the nearest node 
// to the target address
AddressedNode* DeetooNetwork::findShortcutNode(const std::map<my_int, AddressedNode*>& n_map, my_int t_addr) {
    AddressedNode* scNode;
    std::map<my_int, AddressedNode*>::const_iterator it_upper = n_map.upper_bound(t_addr);
    std::map<my_int, AddressedNode*>::const_iterator it_upper1 = it_upper;
    it_upper1--;
    if (it_upper == n_map.begin() || it_upper == n_map.end() )
    {
      if (distanceTo( (n_map.begin() )->first, t_addr) < distanceTo( (n_map.rbegin() )->first, t_addr) ) {
        return (n_map.begin() )->second;
      }
      else {
        return (n_map.rbegin() )->second;
      }
    }
    else {
      if (distanceTo(it_upper->first, t_addr) < distanceTo( it_upper1->first, t_addr) ) {
        return it_upper->second;
      }
      else {
        return it_upper1->second;
      }
    }
}	

my_int DeetooNetwork::distanceTo(my_int addr_a, my_int addr_b) {
    my_int sm, bg, dt;
    sm = std::min(addr_a, addr_b);
    bg = std::max(addr_a, addr_b);
    dt = std::min( (bg-sm),( (WMAX+1) - bg + sm) );
    return dt;
}

void DeetooNetwork::printNetInfo(bool cache) {
   auto_ptr<NodeIterator> ni (getNodeIterator() );
   while ( ni->moveNext() ) {
      AddressedNode* this_node = dynamic_cast<AddressedNode*> ( ni-> current() );
      cout<< "current node's address is: " << this_node->getAddress(cache) << endl;
      if (this->getDegree(this_node) != 0) {
         auto_ptr<NodeIterator> nni (this->getNeighborIterator(this_node) );
	 cout << "neighbors of " << this_node->getAddress(cache) << "are: " << endl;
	 cout << "(";
	 while ( nni->moveNext() ) { 
	    AddressedNode* nei_node = dynamic_cast<AddressedNode*> (nni->current() );
	    cout << nei_node->getAddress(cache) << ",";
         }
	 cout << ")" << endl;
      }
      else {
         cout << "no neighbor" << endl;
      }
   }
}

vector<my_int> DeetooNetwork::getNeighborDist(bool cq) {
  vector<my_int> ret_val;
  my_int this_dist;
  int this_bin;
  auto_ptr<EdgeIterator> ei (getEdgeIterator() );
  while(ei->moveNext() ) {
    Edge* e = ei->current();
    AddressedNode* node_first = dynamic_cast<AddressedNode*> (e->first);
    AddressedNode* node_second = dynamic_cast<AddressedNode*> (e->second);
    this_dist = distanceTo(node_first->getAddress(cq), node_second->getAddress(cq) );
    //cout << this_dist <<endl;
    ret_val.push_back(this_dist);
  }
  return ret_val;
}
my_int DeetooNetwork::guessNetSizeLog(AddressedNode* tnode,bool cq, int con)
{
  my_int log_d = con * (my_int)(log(guessNetSize(tnode,cq)) );
  //cout << "log_d: " << log_d << endl;
  //cout << "nd_map size: " << node_map.size() << "\tquery_nm size: " << query_nm.size() << endl;
  _count_log = 0;  //cost for network size estimation
  std::map<my_int, AddressedNode*>::const_iterator upper;
  upper = node_map.upper_bound(tnode->getAddress(cq) );
  for (int iter = 0; iter < log_d; iter++) {
    if (upper == node_map.end() ) {
      upper = node_map.begin();
    }
    _count_log++;
    //cout << upper->first << endl;
    upper++;
  }

  my_int dist_to = tnode->getDistanceTo(upper->first, cq);
  //cout << dist_to << "\t" << log_d << endl;
  double temp = (double)(log_d) / (double)(dist_to);
  //cout << "temp: " << temp << endl;
  my_int new_est = (my_int)( temp * WMAX);
  //cout << new_est << endl;
  return new_est;

}
std::vector<int> DeetooNetwork::guessNetSizeNeighbors(AddressedNode* tnode, bool cq)
{
  int sum_size1 = 0;
  int sum_size2 = 0;
  int count = 0;
  std::vector<int> my_vec1;
  std::vector<int> my_vec2;
  my_vec1.clear();
  my_vec2.clear();
  auto_ptr<NodeIterator> ni(getNeighborIterator(tnode) );
  while (ni->moveNext() ) {
    AddressedNode* c_node = dynamic_cast<AddressedNode*> (ni->current() );
    int e_size1 = guessNetSize(c_node,cq);
    sum_size1 += e_size1;
    int e_size2 = guessNetSizeLog(c_node,cq,1);
    sum_size2 += e_size2;
    my_vec1.push_back(e_size1);
    my_vec2.push_back(e_size2);
    count++;
  }
  int my_size1 = guessNetSize(tnode,cq);
  int my_size2 = guessNetSizeLog(tnode,cq,1);
  my_vec1.push_back(my_size1);
  my_vec2.push_back(my_size2);
  sum_size1 += my_size1;
  sum_size2 += my_size2;
  count++;
  int mean_1 = (my_int)((double)(sum_size1) / (double)(count));
  int mean_2 = (my_int)((double)(sum_size2) / (double)(count));
  int median_1 = getMedian(my_vec1);
  int median_2 = getMedian(my_vec2);
  std::vector<int> result;
  result.push_back(mean_1);
  result.push_back(mean_2);
  result.push_back(median_1);
  result.push_back(median_2);
  return result;
}
//ask shortcut neighbors to send their log-estimation
int DeetooNetwork::guessNetSizeNeis(AddressedNode* tnode, bool cq)
{
  std::vector<int> my_vec;
  my_vec.clear();
  std::map<my_int, int> my_map;
  my_map.clear();
  auto_ptr<NodeIterator> ni(getNeighborIterator(tnode) );
  while (ni->moveNext() ) {
    AddressedNode* c_node = dynamic_cast<AddressedNode*> (ni->current() );
    my_int c_addr = c_node->getAddress(cq);
    my_int dist = tnode->getDistanceTo(c_addr,cq);
    int e_size = guessNetSizeLog(c_node,cq,1);
    my_map[dist] = e_size;
  }
  int idx = 0;
  _count_median = 0;
  std::map<my_int, int>::iterator it;
  for (it = my_map.begin(); it!=my_map.end(); it++) {
    //excludes direct neighbors
    if (idx >1) {
      my_vec.push_back(it->second);
      _count_median++;
    }
    idx++;
  }
  int my_size = guessNetSizeLog(tnode,cq,1);
  my_vec.push_back(my_size);
  int median = getMedian(my_vec);
  return median;
}

my_int DeetooNetwork::guessNetSize(AddressedNode* tnode,bool cq)
{
  std::map<my_int,AddressedNode*> lefters, righters;
  my_int addr_min = WMAX;
  my_int addr_max = 0;
  my_int this_dist, dist1, dist2;
  my_int this_addr;
  my_int cur_size = node_map.size();
  if (cur_size < 10) {
    return cur_size;
  }
  else {
    //cout << "target Address: " << tnode->getAddress(cq) << endl;
    auto_ptr<NodeIterator> ni(getNeighborIterator(tnode) );
    while (ni->moveNext() ) {
      AddressedNode* c_node = dynamic_cast<AddressedNode*> (ni->current() );
      this_addr = c_node->getAddress(cq);
      if (this_addr < addr_min) { addr_min = this_addr; } //left most neighbor
      if (this_addr > addr_max) { addr_max = this_addr; } //right modst neighbor
      //cout << "current Address: " << c_node->getAddress(cq) << "\tc_dist: " << c_node->getDistanceTo(tnode->getAddress(cq), cq)<< endl;
      // left neighbors
      if (c_node->getAddress(cq) < tnode->getAddress(cq) ) {
   	  this_dist = c_node->getDistanceTo(tnode->getAddress(cq), cq);
	  lefters.insert( make_pair( this_dist, c_node ) );
	  //lefters[this_dist] = c_node;
      }
      // right neighbors
      else {
	  this_dist = c_node->getDistanceTo(tnode->getAddress(cq), cq);
	  righters.insert( make_pair( this_dist, c_node ) );
      }
    }
    // If there are entries in lefters and righters, the first elements in them are direct neighbors.
    if (lefters.size()!=0 && righters.size()!=0) {
      dist1 = lefters.begin()->first;
      dist2 = righters.begin()->first;
    }
    // else, target node is the most right node or the most left node,
    // so, min_addr and max_addr nodes are the direct neighbors
    else {
      dist1 = tnode->getDistanceTo(addr_min, cq);
      dist2 = tnode->getDistanceTo(addr_max, cq);
    }
    lefters.clear();
    righters.clear();
    //cout << "dist1 and dist2: " << dist1 << "\t" << dist2 << endl;
    my_int d_ave = (my_int) ( (double) (dist1 + dist2) / 2.0);
    //cout << "d_ave: " << d_ave << endl;
    my_int d_net = (my_int)( (double) (WMAX / d_ave) + (double) (1 / d_ave) );
    //cout << "d_net: " << d_net << endl;
    return d_net;
  }
}
    
/*
 * createEvenNet is for making network with evenly distributed nodes in address space
 * When a new node join, maximize minimum distance to the neighbors 
 * by picking up two candidate addresses and finally select an address 
 * with longer minimum distance to the neighbors.
 */
void DeetooNetwork::createEvenNet(int net_size) {
  node_map.clear();
  std::set<std::string> items;
  items.clear();
  //add first 2 nodes in the network.
  while(node_map.size() < 2) 
  {
    my_int r_addr = (my_int)(_r_short.getDouble01() * (WMAX) );
    if (node_map.find(r_addr) == node_map.end() && r_addr != 0){
      AddressedNode* anode = new AddressedNode(r_addr, items);
      //cout << "first address: " << r_addr << endl;
      node_map[r_addr] = anode;
      add(anode);
    }
  }
  //cout << "node_map.size(): " << node_map.size() << endl;
  
  //add all the others.
  my_int r_addr1, r_addr2, up_addr, down_addr;
  my_int dist2up, dist2down;
  my_int dist1, dist2;
  std::map<my_int, AddressedNode*>::const_iterator it_up;
  while( node_map.size() < net_size) {
    //cout << "------------------------------" << endl;
    //cout << "node_map.size(): " << node_map.size() << endl;
    //first location
    r_addr1 = (my_int)(_r_short.getDouble01() * (WMAX) ); // The first candidate address
    //cout << "addr1: " << r_addr1 << endl;
    it_up = node_map.upper_bound(r_addr1 ); //The closest upper neighbor of new address
    up_addr =  it_up->first;
    down_addr = (it_up--)->first;  // The closest lower neighbor of new address
    dist2up = distanceTo(up_addr, r_addr1);  // distance to upper neighbor address
    dist2down = distanceTo(down_addr, r_addr1);  // distance to lower neighbor address
    //dist1 = dist2up + dist2down; 
    dist1 = std::min(dist2up , dist2down);  // The first candidate's distance = minimum distance
    //second location
    r_addr2 = (my_int)(_r_short.getDouble01() * (WMAX) );  // The second candidate address
    //cout << "addr2: " << r_addr2 << endl;
    it_up = node_map.upper_bound(r_addr2 );  
    up_addr =  it_up->first;
    down_addr = (it_up--)->first;
    dist2up = distanceTo(up_addr, r_addr2); 
    dist2down = distanceTo(down_addr, r_addr2); 
    //dist2 = dist2up + dist2down; 
    dist2 = std::min(dist2up , dist2down); 
    
    //choose bigger distance
    AddressedNode* new_node;
    my_int new_addr;
    if (dist1 >= dist2) {
      new_node = new AddressedNode(r_addr1,items);
      new_addr = r_addr1;
    }
    else {
      new_node = new AddressedNode(r_addr2,items);
      new_addr = r_addr2;
    }
    //cout << "new addr: " << new_addr << endl;
    if (node_map.find(new_addr) == node_map.end() && new_addr != 0) {
      node_map[new_addr] = new_node;	    
      add(new_node);
    }
  }
  formRing(node_map);
  makeShortcutConnection(node_map, true);
} 

/**
my_int DeetooNetwork::getUniformAddress(int no_can, bool cache) {
  int result_dist = 0;
  my_int ret_addr;
  set<my_int> addr_set;
  my_int r_addr;
  for (int i = 0; i < no_can; i++) {
    r_addr = (my_int)(_r_short.getDouble01() * (WMAX) );
    addr_set.insert(r_addr);
  }
  std::map<my_int, AddressedNode*>::const_iterator it_up;
  set<my_int>::const_iterator set_it;
  my_int up_addr, down_addr;
  int dist2up, dist2down, sum_dist;
  map<my_int,int> distances;
  distances.clear();
  for (set_it=addr_set.begin(); set_it != addr_set.end(); set_it++) {
    it_up = node_map.upper_bound(*set_it);
    up_addr = it_up->first;
    down_addr = (it_up--)->first;
    dist2up = distanceTo(up_addr, *set_it);
    dist2down = distanceTo(down_addr, *set_it);
    sum_dist = dist2up + dist2down;
    distances[r_addr] = sum_dist;
  }
  map<my_int, int>::const_iterator map_it;
  for (map_it = distances.begin(); map_it != distances.end(); map_it++) {
    my_int th_addr = map_it->first;
    int th_dist = map_it->second;
    if (th_dist > result) {
      ret_addr = th_addr;
      result = th_dist;
    }
  }
  return result;
}
*/
/*
 * createEvenNet is for making network with evenly distributed nodes in address space
 * When a new node join, maximize minimum distance to the neighbors 
 * by picking up two candidate addresses and finally select an address 
 * with longer minimum distance to the neighbors.
 */
/**
void DeetooNetwork::createEvenNet(int net_size, my_int new_addr) {
  node_map.clear();
  std::set<std::string> items;
  items.clear();
  //add first 2 nodes in the network.
  while(node_map.size() < 2) 
  {
    my_int r_addr = (my_int)(_r_short.getDouble01() * (WMAX) );
    if (node_map.find(r_addr) == node_map.end() && r_addr != 0){
      AddressedNode* anode = new AddressedNode(r_addr, items);
      //cout << "first address: " << r_addr << endl;
      node_map[r_addr] = anode;
      add(anode);
    }
  }
  //add all the others.
  std::map<my_int, AddressedNode*>::const_iterator it_up;
  while( node_map.size() < net_size) {
    //cout << "------------------------------" << endl;
    //cout << "node_map.size(): " << node_map.size() << endl;
    //first location
    AddressedNode* new_node = new AddressedNode(r_addr1,items);
    //cout << "new addr: " << new_addr << endl;
    if (node_map.find(new_addr) == node_map.end() && new_addr != 0) {
      node_map[new_addr] = new_node;	    
      add(new_node);
    }
  }
  formRing(node_map);
  makeShortcutConnection(node_map, true);
} 
*/

vector<int> DeetooNetwork::getNeighborDistHist(int bins) const {

   vector<int> ret_val;
   int this_nd;
   int this_bin;

   ret_val.resize(bins);
   for (int j = 0; j < bins; j++) {
      ret_val[j] = 0;
   }
   auto_ptr<EdgeIterator> ei( getEdgeIterator() );
   while(ei->moveNext() ) {
      Edge* e = ei->current();
      //if (e->getAttributes() == "SC") {
	 AddressedNode* node_first = dynamic_cast<AddressedNode*> (e->first);
	 AddressedNode* node_second = dynamic_cast<AddressedNode*> (e->second) ;
         this_nd =  node_first->getDistanceTo(this->getNodeSize(), node_second) ;
         this_bin = this_nd * bins;
	 ret_val[this_bin] = ret_val[this_bin] + 1;
      //}
   }
   return ret_val;
}

std::pair<my_int, my_int> DeetooNetwork::getRange(double cq_size) {
  my_int range0, range1;
  //cout << "set ranges==================" << endl;
  my_int start_cr = (my_int)(_r_short.getDouble01()*(double)(AMAX - cq_size - 1 ) );
  double end_cr = (double)start_cr + cq_size-1;
 if (end_cr >= (AMAX-1) ) {
     range1 = WMAX;
     range0 = (my_int)( ( (AMAX-1) - cq_size +1 ) * AMAX);
 } 
 else {
     range0 = start_cr * AMAX;
     range1 = (my_int) ( (end_cr * AMAX) + AMAX -1 );
 }
  return make_pair(range0, range1);
}

int DeetooNetwork::getMedian(std::vector<int> my_vec) {
  int idx;
  int average;
  int size = my_vec.size();
  sort(my_vec.begin(), my_vec.end() );
  //printVector(my_vec);
  if (size % 2) {
    idx = size / 2;
    average = (int)(my_vec[idx]);
  }
  else {
    idx = (int)(size / 2);
    int first = my_vec[idx-1];
    int second = my_vec[idx];
    average = (int)((first+second)/2);
  }
  return average;
}
void DeetooNetwork::printVector(std::vector<int> my_vec) {
  cout << "my_vec: ";
  std::vector<int>::iterator it;
  for (it = my_vec.begin(); it != my_vec.end() ; it++) {
    cout << *it << ", ";
  }
  cout << endl;
}

int DeetooNetwork::brokenEdges(float p) {
  //cout << "initial edge size: " << getEdgeSize() << endl;
  int no_removed = 0;
  auto_ptr<EdgeIterator> ei( getEdgeIterator() );
  while (ei->moveNext() ) {
    Edge* this_edge = ei->current();
    float p_fail = _r_short.getDouble01();
    if ( p_fail < p ) {
      //cout << "p: " << p << " p_fail(ran_no): " << p_fail << endl;
      no_removed += this->remove(this_edge);
      //cout << "no_removed: " << no_removed << endl;
    }
  }
  return no_removed;
}
/*
 * Give some network inconsistency 
 * In a given probability, each node's left neighbor connection is lost.
 * Then, the node establish new connection to the next nearest neighbor on the left side.
 */
void DeetooNetwork::rewireEdges(float p, bool isCache) {
  auto_ptr<NodeIterator> ni (getNodeIterator() );
  while (ni->moveNext() ) {
    AddressedNode* this_node = dynamic_cast<AddressedNode*> (ni->current() );
    /*
    float p_fail = _r_short.getDouble01();
    if (p_fail < p) {
      connectToTheNextLeft(this_node, isCache);
    }
    */
    float p_fail = _r_short.getDouble01();
    if (p_fail < p) {
      connectToTheNextRight(this_node, isCache);
    }
  }
}
/*
 * remove edge to the left node
 *
 * @param this_node a node whose left connection is lost and makes new connection to 
 * the next left node.
 * @param isCache true if caching network, false if querying network
 * 
 */
void DeetooNetwork::connectToTheNextLeft(AddressedNode* this_node, bool isCache) {
  my_int this_addr = this_node->getAddress(isCache);
  std::map<my_int, AddressedNode*>::const_iterator nm_it = node_map.upper_bound(this_addr);
  if ( nm_it != node_map.end() ) { 
    remove(Edge(this_node,nm_it->second) );
    nm_it++;
    if (nm_it != node_map.end() ) {
      add(Edge(this_node,nm_it->second) );
    }
    else { // left node is the last node in the network.
	   // Thus, the first node in the network is new neighbor
      nm_it = node_map.begin();
      add(Edge(this_node,nm_it->second) );
    }
  }
  else { //this_node is the last node in the network.
    nm_it = node_map.begin();
    remove(Edge(this_node,nm_it->second) );
    nm_it++;
    add(Edge(this_node, nm_it->second) );
  }
}
/*
 * remove edge to the right node
 *
 * @param this_node a node whose right connection is lost and makes new connection to 
 * the next right node.
 * @param isCache true if caching network, false if querying network
 * 
 */
void DeetooNetwork::connectToTheNextRight(AddressedNode* this_node, bool isCache) {
  my_int this_addr = this_node->getAddress(isCache);
  std::map<my_int, AddressedNode*>::const_iterator nm_it = node_map.lower_bound(this_addr);
  if ( nm_it != node_map.begin() ) { 
    nm_it--;
    remove(Edge(this_node,nm_it->second) );
    if (nm_it != node_map.begin() ) {
      nm_it--;
      add(Edge(this_node,nm_it->second) );
    }
    else { // right node is the last node in the network.
	   // Thus, the first node in the network is new neighbor
      nm_it = node_map.end();
      nm_it--;
      add(Edge(this_node,nm_it->second) );
    }
  }
  else { //this_node is the last node in the network.
    nm_it = node_map.end();
    nm_it--;
    remove(Edge(this_node,nm_it->second) );
    nm_it--;
    add(Edge(this_node, nm_it->second) );
  }
}

