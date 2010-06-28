#include "exactNodeAct.h"
#include "ran1random.h"
#define FOREACH(it,col) for(it=col.begin();it != col.end();it++)
#ifdef INT64
  typedef unsigned long long my_int;
  #define WMAX 18446744073709551615LL
  #define AMAX 4294967296LL
#else
  typedef unsigned long my_int;
  #define AMAX 65536L
  #define WMAX 4294967295L
#endif
#define DEBUG
#define BOX_M 5
#define BOX_m 2
using namespace Starsky;
using namespace std;

ExactNodeLeaveAction::ExactNodeLeaveAction(EventScheduler& sched, DeetooNetwork& cn, DeetooNetwork& qn, ExactD2Node* me)
      : _sched(sched), _cnet(cn), _qnet(qn), _me(me)
{
}
void ExactNodeLeaveAction::Execute() {
  
  //cout << "-------------NodeLeaveAction-------------- " 
  //	  << _me->getAddress(1) << ", "
  //	  << _me->getAddress(0) << endl;
  
  //schedule a time to remove this node:
  my_int caddr = _me->getAddress(1);
  my_int qaddr = _me->getAddress(0);
  /*
  auto_ptr<NodeIterator> ni (_cnet.getNodeIterator() );
  while (ni->moveNext() ) { 
    ExactD2Node* c = dynamic_cast<ExactD2Node*> (ni->current() );
  }
  */
  /*
  map<my_int, ExactD2Node*>::const_iterator mit;
  for (mit=_cnet.node_map.begin(); mit!=_cnet.node_map.end(); mit++) {
    cout << "c_addr: " << mit->first << endl;
  }
  */
  //remove all edges linked to this node from caching network
  auto_ptr<NodeIterator> nic (_cnet.getNeighborIterator(_me) );
  while (nic->moveNext() ) {
    ExactD2Node* this_node = dynamic_cast<ExactD2Node*> ( nic->current() );
    Edge* this_edge = _cnet.getEdge(_me,this_node);
    _cnet.remove(this_edge );
  }
  //remove all edges linked to this node from caching network
  auto_ptr<NodeIterator> niq (_qnet.getNeighborIterator(_me) );
  while (niq->moveNext() ) {
    ExactD2Node* this_node = dynamic_cast<ExactD2Node*> ( niq->current() );
    Edge* this_edge = _qnet.getEdge(_me,this_node);
    _qnet.remove(this_edge );
  }
  //remove node from caching network's node_map
  _cnet.node_map.erase(_cnet.node_map.find(caddr));
  //remove node from querying network's node_map
  _qnet.node_map.erase(_qnet.node_map.find(qaddr));
  //remove node from caching network
  _cnet.remove(_me);
  //remove node from querying network
  _qnet.remove(_me);

  //Need stabilization here!
  //connect broken ring. make connection between left and right neighbors of leaving node.
  map<my_int, AddressedNode*>::const_iterator c_n_it = _cnet.node_map.upper_bound(caddr);
  map<my_int, AddressedNode*>::const_iterator q_n_it = _qnet.node_map.upper_bound(qaddr);
  ExactD2Node* c_left;
  ExactD2Node* c_right;
  if (c_n_it ==_cnet.node_map.begin() || c_n_it == _cnet.node_map.end() ) {
    // leaving node's address is the biggest or the smallest.
    // the first and the last nodes was it's neighbors.
    // make connection between these two nodes.
    c_left = dynamic_cast<ExactD2Node*> ((_cnet.node_map.begin() )->second);
    c_right = dynamic_cast<ExactD2Node*> ((_cnet.node_map.rbegin() )->second);
  }
  else {
    // leaving node was in the middle of ring.
    c_left = dynamic_cast<ExactD2Node*> (c_n_it->second);
    c_n_it--;
    c_right = dynamic_cast<ExactD2Node*> (c_n_it->second);
  }
  
  ExactD2Node* q_left;
  ExactD2Node* q_right;
  if (q_n_it ==_qnet.node_map.begin() || q_n_it == _qnet.node_map.end() ) {
    q_left = dynamic_cast<ExactD2Node*> (_qnet.node_map.begin()->second);
    q_right = dynamic_cast<ExactD2Node*> (_qnet.node_map.rbegin()->second);
  }
  else {
    q_left = dynamic_cast<ExactD2Node*> (q_n_it->second);
    q_n_it--;
    q_right = dynamic_cast<ExactD2Node*> (q_n_it->second);
  }
  
  _cnet.add(Edge(c_left, c_right));
  _qnet.add(Edge(q_left, q_right));
  /*
#ifdef DEBUG
  std::cout << _sched.getCurrentTime() << "\t"
	    << "Node_Leave\t"
            << _cnet.getNodeSize() << "\t"
            << _cnet.getEdgeSize() << "\t"
	    << _qnet.getNodeSize() << "\t"
	    << _qnet.getEdgeSize() 
            << std::endl;
#endif
*/
}


/**
 * Represents a node joining a network
 * When we execute, we select two random nodes and make edges to them,
 * and schedule a time to leave.
 */
ExactNodeJoinAction::ExactNodeJoinAction(EventScheduler& sched, Random& r, DeetooNetwork& cn, DeetooNetwork& qn) : _sched(sched), _r(r), _cnet(cn), _qnet(qn)
{

  //_boxset.clear();
  _join_cost = 0;
  _no_box = 0;
}

void ExactNodeJoinAction::Execute() {
  //cout << "-------------NodeJoinAction-------------- " << endl;
  ///*
  // Get a random address which is not assigned in the network yet. 
  my_int c_addr = 0;
  my_int q_addr = 0;
  set<string>  items;
  items.clear();
  ExactD2Node* me = 0;
  ExactD2Node* selectednode;
  bool fresh_addr = false;
  while (!fresh_addr) {
    c_addr = (my_int)(_r.getDouble01() * WMAX);
    if (_cnet.node_map.find(c_addr) == _cnet.node_map.end() && c_addr != 0) {
      fresh_addr = true;
    }
    else {
      fresh_addr = false;
    }
  }
  // if 'c_addr' is not selected for address, 
  // 'me' has to be deleted before the end of join.
  me = new ExactD2Node(c_addr, items);
  if (_cnet.node_map.size() < 1)
  {
    selectednode = me;
    Box* box = new Box(0,(AMAX-1),0, (AMAX-1) );
    _no_box += 1;
    selectednode->setBox(box);
    box->addNode(selectednode);
    //cout << "first node is added" << endl;
  }
  else {
    // There are at least 1 node in the network. add more here
    // let's make a temporary network that contains neighbor nodes both in cnet and in qnet.
    set<ExactD2Node*> neighborset;
    neighborset.clear();
    getNeighbors(_cnet, me, true, neighborset);
    getNeighbors(_qnet, me, false, neighborset);
    _join_cost += neighborset.size();
    //cout << "@@@@@@@@@@@@@@@@@@@@nei size: " <<neighborset.size() << endl;

    ExactD2Node* nei;
    Box* this_box;
    set<ExactD2Node*>::const_iterator nei_it;
    for (nei_it = neighborset.begin(); nei_it != neighborset.end(); nei_it++) {
      nei = dynamic_cast<ExactD2Node*> (*nei_it);
      this_box = nei->getBox();
      if (this_box->inBox(me) ) {
	//cout << "find a neighbor sharing box" << endl;
	break;
      } 
      else {
        continue;
      }
    }

    q_addr = me->getAddress(false);
    //cout << "cnode: " << c_addr << ", nei: " << nei->getAddress(1) << ", in box? " << this_box->inBox(me) << endl;
    //cout << "qnode: " << q_addr << ", nei: " << nei->getAddress(0) << ", in box? " << this_box->inBox(me) << endl;

    bool moreCols = nei->getBox()->splitColumn(); // true if column needs to be split.

    //cout << "box size: " << this_box->count() << endl;
    if ( this_box->count() >= BOX_M ) {
      //Let's find a box with mininum number of nodes in the column or row.
      // Bounded broadcasting over column or row.
      // Lower number of columns or rows will be selected.
      //cout << "if > M --------------------------------" << endl;
      pair<int, pair<Box*, ExactD2Node*> > min_box;
      //cout << "--------------------------------" << endl;
      my_int start, end;
      if (moreCols) {
        //broadcast over columns (caching network)
        pair<my_int, my_int> range = this_box->getBroadcastRange(true);
	start = range.first;
	end = range.second;
	//cout << "for cache net, min box range is: " << start << ", " << end << endl;
        min_box = getBoxMin(_cnet, nei, start, end, true);
      }
      else {
        //broadcast over rows (querying network)
        pair<my_int, my_int> range = this_box->getBroadcastRange(false);
	start = range.first;
	end = range.second;
	//cout << "for query net, min box range is: " << start << ", " << end << endl;
        min_box = getBoxMin(_qnet, nei, start, end, false);
      }

      pair<Box*, ExactD2Node*> box_info = min_box.second;
      Box* box = box_info.first;
      //cout << "N(min_box): " << min_box.first << endl;
      if (min_box.first < BOX_M) {
        //cout << "min box is not full" << endl;
	// number of nodes in min_box is smaller than maximum.
	// new node should join in this box at splittable position
        // The node can be join in a proper position in the min_box.
	my_int addr = box->getJoinAddress(_r);
	selectednode = new ExactD2Node(addr, items);
	box->addNode(selectednode);
	selectednode->setBox(box);
	//cout << "new addr: " << addr << " is in box? " << box->inBox(selectednode) << ", box count: " << box->count() << endl;
	delete me;
      }
      else {
        // min_box is also full
	// this column or row needs to be split.
	//
        //cout << "min box is  full            SPLIT!!!!!!!!!!!!1" << endl;
        //cout << "column split? " << moreCols << endl;
	string position = (nei->getBox())->getPosition(nei);
	
	/*
	Box* tmp_box = nei->getBox();
	vector<my_int> bound = tmp_box->getBoundary();
	my_int bound_start_c = bound[0];
	my_int bound_end_c = bound[1];
	my_int bound_start_q = bound[2];
	my_int bound_end_q = bound[3];
	cout << "before split: boundary: " << bound_start_c << ":" << bound_end_c << ", " << bound_start_q << ":" << bound_end_q << endl; 
	//cout << "for split, range is: " << start << ", " << end << endl;
	*/
	/*
	if (nei->getBox()->isSplittable() ) { 
          //cout << " splittable " << endl; 
	} 
	*/
	//Box* my_box = split(_cnet, nei, start, end, moreCols);
	Box* my_box;
	if (moreCols) {
	  my_box = split(_cnet, nei, start, end, moreCols);
	}
	else {
          my_box = split(_qnet, nei, start, end, moreCols);
	}


	/*
	vector<my_int> bound1 = my_box->getBoundary();
	my_int bound_start_c1 = bound1[0];
	my_int bound_end_c1 = bound1[1];
	my_int bound_start_q1 = bound1[2];
	my_int bound_end_q1 = bound1[3];
	cout << "after split: boundary: " << bound_start_c1 << ":" << bound_end_c1 << ", " << bound_start_q1 << ":" << bound_end_q1 << endl; 
	*/
	// network is splitted
	// join with c_addr in splitted box at a proper position
	// box is this_box
	// node is me
	//if (lu->count() >= rb->count() ) {
	//cout << "network is splitted" << endl;
        my_int addr = my_box->getJoinAddress(_r);
	selectednode = new ExactD2Node(addr, items);
	my_box->addNode(selectednode);
	selectednode->setBox(my_box);
	/*
	cout << "----------------" << endl;
	cout << "my_box count: " << my_box->count() << endl;
	cout << "selectednode's box: " << selectednode->getBox() << endl;
	cout << "selectednode's adr: " << selectednode->getAddress(1) << ":" << selectednode->getAddress(0) << endl;
	cout << "selectednode is in Box? " << my_box->inBox(selectednode) << endl;
	*/
	delete me;
	//cout << "----------------" << endl;
        
      }

    }
    else {
      //The box is not yet full.
      //Just join in this box with c_addr.
      //cout << " N(min_box) < M : box is not full yet" << endl;
      my_int addr = this_box->getJoinAddress(_r);
      //cout << "new addr is "<< addr << endl;
      delete me;
      selectednode = new ExactD2Node(addr, items);
      this_box->addNode(selectednode);
      selectednode->setBox(this_box);
    }
  }


  //cout << "--------------just created, item size? " << me->getObject().size() << endl;
  //my_int q_addr = me->getAddress(0);
  c_addr = selectednode->getAddress(true);
  q_addr = selectednode->getAddress(false);
  
  getConnection(_cnet, selectednode, true);
  getConnection(_qnet, selectednode, false);
  _cnet.add(selectednode);
  _qnet.add(selectednode);
  _cnet.node_map[c_addr] = selectednode;
  _qnet.node_map[q_addr] = selectednode;
  //printNetworkInfo(_cnet, true);
  //printNetworkInfo(_qnet, false);
  //cout << "------------ after add to nodemap, csize: " << _cnet.node_map.size() << ", qsize: " << _qnet.node_map.size() << endl;
  //cout << "---------connected, item size? " << me->getObject().size() << endl;
  /*
  map<Box*, my_int> cols;
  map<Box*, my_int> rows;
  cols.clear();
  rows.clear();
  my_int col_width;
  my_int row_width;
  cout << "________________________________________" << endl;
  auto_ptr<NodeIterator> nb ( _cnet.getNodeIterator() );
  while ( nb->moveNext() ) {
    ExactD2Node* en = dynamic_cast<ExactD2Node*> ( nb->current() );
    my_int c_addr = en->getAddress(1);
    my_int q_addr = en->getAddress(0);
    Box* box = en->getBox();
    vector<my_int> bound = box->getBoundary();
    //cout << bound.size() << endl;
    //cout << "@@@@@node: (" << c_addr << ": " << q_addr << "), size: " << box->count() << ", box: " << box << ", box range: " << bound[0] << ", " << bound[1] << ", " << bound[2] << ", " << bound[3]  << endl;
    //cout << "c_diff: " << bound[1] - bound[0] << ", q_diff: " << bound[3] - bound[2] << endl;
    col_width = bound[1] - bound[0];
    row_width = bound[3] - bound[2];
    cols[box] = col_width;
    rows[box] = row_width; 
  }
  cout << "cols: " << cols.size() << ", rows: " << rows.size() << endl;
  */
  /*
  set<my_int> cols;
  set<my_int> rows;
  cols.clear();
  rows.clear();
  cout << "-------------------------------------------------" << endl;
  auto_ptr<NodeIterator> nb ( _cnet.getNodeIterator() );
  while ( nb->moveNext() ) {
    ExactD2Node* en = dynamic_cast<ExactD2Node*> ( nb->current() );
    my_int c_addr = en->getAddress(1);
    my_int q_addr = en->getAddress(0);
    cout << "++++: " << endl;
    Box* box = en->getBox();
    cout << "++++: " << endl;
    vector<my_int> bound = box->getBoundary();
    cout << bound.size() << endl;
    cout << "@@@@@node: (" << c_addr << ": " << q_addr << "), size: " << box->count() << ", box: " << box << ", box range: " << bound[0] << ", " << bound[1] << ", " << bound[2] << ", " << bound[3]  << endl;
    cout << "c_diff: " << bound[1] - bound[0] << ", q_diff: " << bound[3] - bound[2] << endl;
    cols.insert(bound[0]);
    cols.insert(bound[1]);
    rows.insert(bound[2]);
    rows.insert(bound[3]);
    cout << "cols: " << cols.size() << ", rows: " << rows.size() << endl;
  } 
 */ 
  /*
  //Plan to leave:
  //double lifetime = 3600.0 * _r.getDouble01();
  // lifetime and sleep time: exponentially distributed
  double lifetime = _r.getExp(3600.0);
  Action* leave = new NodeLeaveAction(_sched, _cnet, _qnet, selectednode);
  _sched.after(lifetime, leave);
  //Plan to rejoin
  //double sleeptime = 3600.0 * _r.getDouble01();
  double sleeptime = _r.getExp(3600.0);
  Action* rejoin = new NodeJoinAction(_sched, _r, _cnet, _qnet);
  _sched.after(lifetime + sleeptime, rejoin);
  */
  //Print out results:
  /*
#ifdef DEBUG
  std::cout << "join\t" 
            << _sched.getCurrentTime() << "\t"
            << _cnet.getNodeSize() << "\t"
            << _cnet.getEdgeSize() << "\t"
            << _qnet.getNodeSize() << "\t"
            << _qnet.getEdgeSize() << "\t"
            << _join_cost << "\t"
	    //<< stabilization_msgs 
	    << std::endl;
#endif
*/
}
pair<int, pair<Box*, ExactD2Node*> > ExactNodeJoinAction::getBoxMin(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end, bool isCache) {
  auto_ptr<DeetooMessage> m (new DeetooMessage(start, end, isCache, _r, 0) );
  //cout << "--------------------------------------- visit for get min box ---------------------" << endl;
  auto_ptr<DeetooNetwork> visited_net( m->visit(node, net) );
  //cout << "--------------------------------------- visit finished ---------------------" << endl;
  _join_cost += visited_net->getNodeSize();
  auto_ptr<NodeIterator> nit (visited_net->getNodeIterator() );
  int box_min = BOX_M;
  int this_min;
  Box *min_box, *current_box;
  ExactD2Node* this_node;
  pair<Box*, ExactD2Node*> result; 
  while (nit->moveNext() ) {
    ExactD2Node* node = dynamic_cast<ExactD2Node*> (nit->current() );
    current_box = node->getBox();
    this_min = current_box->count();
    //cout << "this node: " << node->getAddress(isCache) << endl;
    if (this_min < BOX_M) {
      box_min = this_min;
      min_box = current_box;
      this_node = node;
    }
  }
  //cout << "box min: " << box_min << endl;
  result = make_pair(min_box, this_node);
  return make_pair(box_min, result);
  //return result;
}
Box* ExactNodeJoinAction::split(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end, bool isColumn) {
  auto_ptr<DeetooMessage> m (new DeetooMessage(start, end, isColumn, _r, 0) );
  //cout << "*******************************in split method" << endl;
  //cout << "column: " << isColumn << "\t node: " << node->getAddress(isColumn) << "\trange: " << start << ", " << end << endl; 
  auto_ptr<DeetooNetwork> visited_net( m->visit(node, net) );
  //cout << "*******************************" << endl;
  //cout << "visited net size(in split method): " << visited_net->getNodeSize() << endl; 
  _join_cost += visited_net->getNodeSize();
  auto_ptr<NodeIterator> nit (visited_net->getNodeIterator() );
  //cout << "*******************************" << endl;
  pair<my_int, my_int>  ele_range = (node->getBox())->getAddrOfElement(isColumn);
  my_int past_ele_diff = ele_range.second - ele_range.first;
  //cout << "this node's box: " << node->getBox() << endl; 
  //cout << "range: " << ele_range.first << ":" << ele_range.second << ", diff: " << past_ele_diff << endl;
  Box *current;  
  Box *ret_box;
  bool inBox = false; //inBox will be set true if node is in a box
                      //then splitBox will return
		      // new splitted box for 'node'.

  int it_no = 0;
  //cout << "visited_net size: " << visited_net->getNodeSize() << endl;
  while (nit->moveNext() ) {
    ExactD2Node* node = dynamic_cast<ExactD2Node*> (nit->current() );
    current = node->getBox();
    if (current->inBox(node) ) {
      inBox = true;
    }
    //cout << "in while this box: " << current << endl;
    pair<my_int, my_int> this_elerange = current->getAddrOfElement(isColumn);
    my_int ele_diff = this_elerange.second - this_elerange.first;
    //cout << "elements: " << this_elerange.first << ":" << this_elerange.second << ", diff: " << ele_diff << endl;
    /*
    my_int e_start = this_elerange.first;
    my_int e_end = this_elerange.second;
    */
    //cout << "-------------------------------------" << endl;
    //cout << "iter_no: " << it_no << ", current_node: " << node->getAddress(isColumn) << endl;

    /*
      my_int mid = current->getMiddle(isColumn);
      node->addColRow(mid, isColumn);
      */
    //if (it_no == 0 || (e_start == past_start_ele && e_end == past_end_ele) ) {
    if (it_no == 0 || (ele_diff >= (past_ele_diff-2) ) ) {
      // first node accessed by split()
      // or box is not yet splitted by other node.
      // split this box anyway
      //cout << " need to be splitted" << endl;
      Box* tmp = current->splitBox(isColumn,inBox);
      //cout << "box: " << current << " will be deleted" << endl;
      delete current;
      if (tmp != 0) {
        ret_box = tmp;
      }
      else {
        //cout << " return box is NULL!!!!!!!!!!!!!!!!" << endl;
      }

      //cout << "splitted," << endl; 
    }
    else {
      //already splited
      //cout << "already splited" << endl;
      //cout << "ele_diff: " << ele_diff << ", past_ele_diff: " << past_ele_diff << ", 2* ele_diff: " << 2*ele_diff << endl;
    }
    /*
    if (current->inBox(node) ) {
      ret_box = current; 
    }
    */
    it_no++;
  }
  return ret_box;

}
void ExactNodeJoinAction::getNeighbors(DeetooNetwork& net, ExactD2Node* n, bool isCol, set<ExactD2Node*> &nei_set) {
  ExactD2Node* neighbor0;
  ExactD2Node* neighbor1;
  my_int addr = n->getAddress(isCol);
  map<my_int, AddressedNode*>::const_iterator it = net.node_map.upper_bound(addr);
  if (it == net.node_map.end() || it == net.node_map.begin() ) {
    neighbor1 = dynamic_cast<ExactD2Node*> ((net.node_map.begin() )->second);
    neighbor0 = dynamic_cast<ExactD2Node*> ((net.node_map.rbegin() )->second);
  }
  else {
    neighbor0 = dynamic_cast<ExactD2Node*> (it->second);
    it--;
    neighbor1 = dynamic_cast<ExactD2Node*> (it->second);
  }
  //cout << "neighbors " << neighbor0->getAddress(isCol) << ", " << neighbor1->getAddress(isCol) << endl;
  nei_set.insert(neighbor0);
  nei_set.insert(neighbor1);
  //cout << "nei size: " << nei_set.size() << endl;

}

int ExactNodeJoinAction::copyObjects(ExactD2Node* me, ExactD2Node* nei) {
    map<string, pair<my_int, my_int> > so = nei->getObject();
    map<string, pair<my_int, my_int> >::iterator so_it;
    int stab_cost = 0; //stabilization cost: count how many object are copied.
    for (so_it = so.begin(); so_it != so.end(); so_it++) {
      my_int adr = me->getAddress(true);
      //cout << "adr: " << adr << "\tstart: " << so_it->second.first << "\tend: " << so_it->second.second << endl;
      if (adr >= so_it->second.first && adr <= so_it->second.second) {
	string str = so_it->first;
	if (!me->searchObject(str) ) {
	  //cout << "yes, insert!!!" << endl;
	  string this_str = so_it->first;
          me->insertObject(this_str, so_it->second.first, so_it->second.second);
	  stab_cost++;
        }
      }
      else { // this node is out of range of object. do not cache it 
	//cout << "++++++++++++++++++++++++  out of range ------------" << endl;      
      }
    }
    return stab_cost;
}
void ExactNodeJoinAction::printNetworkInfo(Network& net, bool cache) {
  //cout << "--------------------------node--------------------" << endl;
  DeetooNetwork& dnet = dynamic_cast<DeetooNetwork&> (net);
  auto_ptr<NodeIterator> nit ( net.getNodeIterator() );
  while (nit->moveNext() ) {
    ExactD2Node* n = dynamic_cast<ExactD2Node*>(nit->current() );
    cout << n->getAddress(cache) << ", "; 
  }
  cout << endl;
  cout << "nodemap " << endl;
  map<my_int, AddressedNode*>::const_iterator nm;
  for (nm = dnet.node_map.begin(); nm != dnet.node_map.end(); nm++) {
    cout << nm->first << ", " ;
  }
  cout << endl;

  cout << "--------------------------edge--------------------" << endl;
  auto_ptr<EdgeIterator> ei ( net.getEdgeIterator() );
  while (ei->moveNext() ) {
    Edge* e = ei->current();
    cout << e->toString(cache) << ", "; 
  }
  cout << endl;
}
void ExactNodeJoinAction::getConnection(DeetooNetwork& net, ExactD2Node* me, bool cache)
{
  my_int addr = me->getAddress(cache);
  /*
  cout << "====================== connection ===========================" << endl;
  cout << "addr: " << addr << ", nodemap.size(): " << net.node_map.size() << endl;
  map<my_int, AddressedNode*>::const_iterator ttt;
  for (ttt = net.node_map.begin(); ttt != net.node_map.end(); ttt++) {
    cout << ttt->first << ", ";
  }
  cout << endl;
  */
  // make connections: ring connection as well as shortcut connections
  net.add(me);
  if (net.getNodeSize() == 0) {
    // *me* is the only node in this network
    // no connection is needed.
  }
  // There is only one node in the network. Make connection with it.
  else if ( net.getNodeSize() == 1) {
    ExactD2Node* neighbor = dynamic_cast<ExactD2Node*> (net.node_map.begin()->second);
    if(neighbor != 0 && neighbor != me) {
      net.add(Edge(me, neighbor));
      //cout << "connection to " << neighbor->getAddress(1);
    }
  }
  else {
    map<my_int, AddressedNode*>::const_iterator cit = net.node_map.upper_bound(addr);
    ExactD2Node* neighbor0;
    ExactD2Node* neighbor1; 
    if (cit == net.node_map.end() || cit == net.node_map.begin() ) { //my node has the biggest or smallest address.
      //cout << "biggest or smallest" << endl;
      neighbor1 = dynamic_cast<ExactD2Node*> ((net.node_map.begin())->second);
      neighbor0 = dynamic_cast<ExactD2Node*> ((net.node_map.rbegin())->second); 
      //cout << neighbor1->getAddress(cache) << ", " << neighbor0->getAddress(cache) << endl; 
    }
    else {  //my node has an address between min and max address
      neighbor0 = dynamic_cast<ExactD2Node*> (cit->second);
      cit--;
      neighbor1 = dynamic_cast<ExactD2Node*> (cit->second);
      //net.makeShortcutConnection(net.node_map, cache);
      //cout << neighbor1->getAddress(cache) << ", " << neighbor0->getAddress(cache) << endl; 
    }
    //remove edge between neighbor0 and neighbor1 if it exists
    Edge* old_edge = net.getEdge(neighbor0,neighbor1);
    if (old_edge != 0) {
      //ExactD2Node* no0 = dynamic_cast<ExactD2Node*> (old_edge->first);
      //ExactD2Node* no1 = dynamic_cast<ExactD2Node*> (old_edge->second);
      //cout << "old_edge: " << neighbor0->getAddress(cache)
      //     << " : " << neighbor1->getAddress(cache) << endl;
      net.remove(old_edge);
    }
    ExactD2Node* shortcut = dynamic_cast<ExactD2Node*> (net.returnShortcutNode(me, net.node_map,true) );
    //make connection to the three neighbors
    if(!(net.getEdge(me, neighbor0)) && !(net.getEdge(neighbor0, me)) && neighbor0 != me) {
      net.add(Edge(me, neighbor0));
      //cout << "connection to " << neighbor0->getAddress(1);
    }
    if(!(net.getEdge(me, neighbor1)) && !(net.getEdge(neighbor1, me)) && neighbor1 != me) {
      net.add(Edge(me, neighbor1));
      //cout << "connection to " << neighbor1->getAddress(1);
    }
    if(!(net.getEdge(me, shortcut)) && !(net.getEdge(shortcut, me)) && shortcut != me) {
      net.add(Edge(me, shortcut));
      //cout << "connection to " << shortcut->getAddress(1);
    }

    /*
    if (cache) {
      // copy objects from my new neighbors if their range includes my address.
      int cost0 = copyObjects(me,neighbor0);
      int cost1 = copyObjects(me,neighbor1);
      //stabilization_msgs = cost0 + cost1 + cost2;
      stabilization_msgs = cost0 + cost1;
      //int cost2 = copyObjects(me,shortcut);
      double guess = _cnet.guessNetSizeLog(me,1,1);
      //cout << "SIZE:::::guess size : " << guess << ", actual size: " << _cnet.node_map.size() << endl;
      double cqsize = (double) (((AMAX) / (double)sqrt(guess ) ) * _sq_alpha);
      //cout << "CQ:: " << cqsize << endl;
      //delete out of range objects from objects list.
      me->stabilize(cqsize);
    }
    */

    //net.node_map[addr] = me;
    //make sure ring is closed.
    ExactD2Node* front = dynamic_cast<ExactD2Node*> (net.node_map.begin()->second);
    ExactD2Node* back = dynamic_cast<ExactD2Node*> ((net.node_map.rbegin())->second);
    if(!(net.getEdge(front,back)) && !(net.getEdge(back, front)) && back != front) {
      net.add(Edge(front,back));
    }
    //cout << "after edge addition: " << net.getEdgeSize() << endl;
  }
}

ExactCacheAction::ExactCacheAction(EventScheduler& sched, Random& r, INodeSelector& ns, DeetooNetwork& n, string& so) : _sched(sched), _r(r), _ns(ns), _net(n), _so(so)
{
  
}
void ExactCacheAction::Execute() {
  //cout << "cacheaction start here" << endl;
  //schedule a time to cache object to nodes in the range:
  _ns.selectFrom(&_net);
  ExactD2Node* node = dynamic_cast<ExactD2Node*> (_ns.select() );

  Box* this_box = node->getBox();
  pair<my_int,my_int> range = this_box->getBroadcastRange(true);
  my_int rg_start = range.first;
  my_int rg_end = range.second;
  /*
  vector<my_int> bound = this_box->getBoundary();
  cout << "~~~~~~~~~~~~~~~~~~~~~~caching addr: " << node->getAddress(true) << endl;
  pair<my_int, my_int> colrow = this_box->addrToColRow(node->getAddress(true) );
  cout << "this node's row: " << colrow.first << endl;
  cout << "caching bound: " << bound[0] << ":" << bound[1] << endl;
  cout << "caching range: " << range.first << ":" << range.second << endl;
  */
  auto_ptr<DeetooMessage> cache_m(new DeetooMessage(rg_start, rg_end, true, _r, 0.0) );
  auto_ptr<DeetooNetwork> tmp_net (cache_m->visit(node, _net));
  auto_ptr<NodeIterator> ni (tmp_net->getNodeIterator() );
  while (ni->moveNext() ) {
    ExactD2Node* inNode = dynamic_cast<ExactD2Node*> (ni->current() );
    inNode->insertObject(_so, rg_start, rg_end);
  }
  /*
  node->stabilize(cqsize);
  */
#ifdef DEBUG
  std::cout << "caching\t"
            << _sched.getCurrentTime() << "\t"
            << _net.getNodeSize() << "\t"
	    << tmp_net->getNodeSize() << "\t"
	    << tmp_net->getDistance(cache_m->init_node) << "\t"
	    << _so
	    << std::endl;
#endif
}
ExactQueryAction::ExactQueryAction(EventScheduler& sched, Random& r, INodeSelector& ns, DeetooNetwork& n, string so) : _sched(sched), _r(r), _ns(ns), _net(n), _so(so)
{

}
void ExactQueryAction::Execute() {
  //schedule a time to query object to nodes in the range:
  //cout << "queryaction start here" << endl;
  UniformNodeSelector u_node(_r);
  _ns.selectFrom(&_net);
  ExactD2Node* node = dynamic_cast<ExactD2Node*> (_ns.select() );

  Box* this_box = node->getBox();
  pair<my_int, my_int> range = this_box->getBroadcastRange(0);
  my_int rg_start = range.first;
  my_int rg_end = range.second;
  vector<my_int> bound = this_box->getBoundary();
  //cout << "~~~~~~~~~~~~~~~~~~~~~~querying addr: " << node->getAddress(false) << endl;
  /*
  pair<my_int,my_int> start_colrow = this_box->addrToColRow(rg_start);
  cout << "rg_start's row: " << start_colrow.first << endl;
  pair<my_int, my_int> end_colrow = this_box->addrToColRow(rg_end);
  cout << "rg_end's row: " << end_colrow.first << endl;
  pair<my_int, my_int> colrow = this_box->addrToColRow(node->getAddress(true) );
  */
  /*
 cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ in query action~~~~~~~~" << endl;
  cout << "box is " << this_box << endl;
  cout << "this node's row: " << colrow.second << endl;
  cout << "querying bound: " << bound[2] << ":" << bound[3] << endl;
  cout << "cache bound: " << bound[0] << ":" << bound[1] << endl;
  cout << "querying range: " << range.first << ":" << range.second << endl;
  cout << "this_box: " << this_box << endl;
  cout << "this box's nodes: " << this_box->count() << endl;
  cout << "this node: " << node->getAddress(0) << ", range: " << rg_start << ":" << rg_end << endl;
  */
  auto_ptr<DeetooMessage> query_m (new DeetooMessage(rg_start, rg_end,false, _r, 0.0) );
  auto_ptr<DeetooNetwork> tmp_net (query_m->visit(node, _net));
  no_msg = tmp_net->getNodeSize();
  depth = tmp_net->getDistance(query_m->init_node);
  sum_hits = 0;
  auto_ptr<NodeIterator> ni (tmp_net->getNodeIterator() );
  while (ni->moveNext() ) {
    ExactD2Node* inNode = dynamic_cast<ExactD2Node*> (ni->current() );

    //if (_so == 0) { cout << "null" << endl; }
    //else { cout << "not null" << endl; }
    //cout << "# objects: " << inNode->getObject().size() << endl;
    sum_hits += inNode->searchObject(_so);
  } 
  //double hit_rate = (double)sum_hits / (double)no_msg;
#ifdef DEBUG
  std::cout << "querying\t"
            << _sched.getCurrentTime() << "\t"
            << _net.getNodeSize() << "\t"
	    << no_msg << "\t"
            << depth << "\t"
	    << _so << "\t" 
	    << sum_hits
	    //<< hit_rate << "\t"
	    << std::endl;
#endif
}




