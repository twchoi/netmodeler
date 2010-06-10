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
  auto_ptr<NodeIterator> ni (_cnet.getNodeIterator() );
  while (ni->moveNext() ) { 
    ExactD2Node* c = dynamic_cast<ExactD2Node*> (ni->current() );
  }
  /*
  map<my_int, ExactD2Node*>::const_iterator mit;
  for (mit=_cnet.node_map.begin(); mit!=_cnet.node_map.end(); mit++) {
    cout << "c_addr: " << mit->first << endl;
  }
  */
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
      //cout << "in while addr: " << me->getAddress(1) << endl;
      fresh_addr = true;
    }
    else {
      fresh_addr = false;
    }
  }
  // if 'c_addr' is not selected for address, 
  // 'me' has to be deleted before the end of join.
  me = new ExactD2Node(c_addr, items);
  int size = _cnet.node_map.size();
  cout << "size: " << size << endl;
  if (_cnet.node_map.size() < 1)
  {
    // Network has only one node.
    // Make new Box which covers whole address space.
    //Box* first_box = new Box(0,WMAX);
    set<my_int> cols;
    set<my_int> rows;
    cols.insert(0);
    cols.insert(AMAX);
    rows.insert(0);
    rows.insert(AMAX);
    selectednode = me;
    Box* box = new Box(0,WMAX);
    selectednode->setBox(box);
    box->addNode(selectednode,_cnet);
    selectednode->updateCols(cols);
    selectednode->updateRows(rows);
    //me->
  }
  else {
    cout << " m < size < M? " << endl;
    map<my_int, AddressedNode*>::const_iterator cit = _cnet.node_map.upper_bound(c_addr);
    set<my_int> cols;
    set<my_int> rows;
    ExactD2Node* neighbor0;
    ExactD2Node* neighbor1;
    ExactD2Node* nei;
    if (cit == _cnet.node_map.end() || cit == _cnet.node_map.begin() ) {
      neighbor1 = dynamic_cast<ExactD2Node*> ((_cnet.node_map.begin() )->second);
      neighbor0 = dynamic_cast<ExactD2Node*> ((_cnet.node_map.rbegin() )->second);
    }
    else {
      neighbor0 = dynamic_cast<ExactD2Node*> (cit->second);
      cit--;
      neighbor1 = dynamic_cast<ExactD2Node*> (cit->second);
    }
    cout << "111111111111111111111111" << endl;
    Box* box0 = neighbor0->getBox();
    Box* box1 = neighbor1->getBox();
    /* debugging */
    pair<my_int,my_int> bound0 = box0->getBoundary();
    pair<my_int,my_int> bound1 = box1->getBoundary();
    cout << "box0's boundary: (" << bound0.first << ", " << bound0.second << ")" << endl;
    cout << "box1's boundary: (" << bound1.first << ", " << bound1.second << ")" << endl;
    cout << "111111111111111111111111" << endl;
    cout << "c_addr: " << c_addr << endl;
    Box* this_box;
    if (box0->inBox(c_addr) ) {
      cout << "inbox" << endl;
      this_box = box0;
      nei = neighbor0;
    }
    else if (box1->inBox(c_addr) ) {
      cout << "outbox" << endl;
      this_box = box1;
      nei = neighbor1;
    }
    else {
      //c_addr is not in the neighbor1's box nor the neighbor0's box.
      //This is impossible!!!
      cout << "not in nor out of box" << endl;
    }

    cout << "111111111111111111111111" << endl;
    cols = nei->getCols();
    rows = nei->getRows();
    bool moreCols = (cols.size() >= rows.size() );
    cout << "222222222222222222" << endl;

    if ( this_box->count() >= BOX_M ) {
      //Let's find a box with mininum number of nodes in the column or row.
      // Bounded broadcasting over column or row.
      // Lower number of columns or rows will be selected.
      pair<int, pair<Box*, ExactD2Node*> > min_box;
      pair<my_int,my_int> boundary = (nei->getBox() )->getBoundary();
      my_int box_start = boundary.first;
      my_int box_end = boundary.second;
      my_int start = box_start * AMAX;
      my_int end = box_end * AMAX + AMAX -1;
      if (moreCols) {
        //broadcast over columns (caching network)
      cout << "~33333333333333333333" << endl;
        min_box = getBoxMin(_cnet, me, start, end);
      cout << "~44444444444444444444" << endl;
      }
      else {
        //broadcast over rows (querying network)
      cout << "33333333333333333333" << endl;
        min_box = getBoxMin(_qnet, me, start, end);
      cout << "44444444444444444444" << endl;
      }

      if (min_box.first < BOX_M) {
	// number of nodes in min_box is smaller than maximum.
	// new node should join in this box at splittable position
	pair<Box*, ExactD2Node*> box_info = min_box.second;
	Box* box = box_info.first;
        // The node can be join in a proper position in the min_box.
	my_int addr = box->getJoinAddress(_r);
	selectednode = new ExactD2Node(addr, items, cols, rows);
	delete me;
      }
      else {
        // min_box is also full
	// this column or row needs to be split.
	//
	// ---------------------------------------------
	// cols and row need to be updated!!!!!!!!!!!!!!
	// ---------------------------------------------
	//
	string position = (me->getBox())->getPosition(me);
	bool isLoU; // left if column, upper if row
	if (moreCols && ( position == "lu" || position == "lb") ) {
          isLoU = true;
	}
	else { isLoU = false; }
	if (!moreCols && ( position == "lu" || position == "ru") ) {
          isLoU = true;
	}
	else { isLoU = false; }
	
	if (moreCols) {
	  split(_cnet, me, start, end, moreCols, isLoU);
	}
	else {
	  split(_qnet, me, start, end, moreCols, isLoU);
	}
	// network is splitted
	// join with c_addr in splitted box at a proper position
	// box is this_box
	// node is me
        my_int addr = this_box->getJoinAddress(_r);
	selectednode = new ExactD2Node(addr, items, cols, rows);
	delete me;
        
      }
      cout << "222222222222222222222" << endl;

    }
    else {
      //The box is not yet full.
      //Just join in this box with c_addr.
      cout << "not full" << endl;
      my_int addr = this_box->getJoinAddress(_r);
      cout << "---------------" << endl;
      cout << "size of cols: " << cols.size() << ", size of rows: " << rows.size() << ", size of items: " << items.size() << endl;
      delete me;
      cout << "---------------" << endl;
      cout << "addr: " << addr << endl;
      selectednode = new ExactD2Node(addr, items);
      this_box->addNode(selectednode, _cnet);
      selectednode->setBox(this_box);
      cout << "---------------" << endl;
    }
  }


  //cout << "--------------just created, item size? " << me->getObject().size() << endl;
  //my_int q_addr = me->getAddress(0);
  getConnection(_cnet, selectednode, true);
  getConnection(_qnet, selectednode, false);
  //Make sure I get added no matter what
  _cnet.add(selectednode);
  _qnet.add(selectednode);
  //add me to each network's node_map
  _cnet.node_map[c_addr] = selectednode;
  _qnet.node_map[q_addr] = selectednode;
  //cout << "---------connected, item size? " << me->getObject().size() << endl;
  
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
  std::cout << _sched.getCurrentTime() << "\t"
	    << "Node_Join\t" 
            << _cnet.getNodeSize() << "\t"
            << _cnet.getEdgeSize() << "\t"
            << _qnet.getNodeSize() << "\t"
            << _qnet.getEdgeSize() << "\t"
	    << stabilization_msgs 
	    << std::endl;
#endif
*/
}
pair<int, pair<Box*, ExactD2Node*> > ExactNodeJoinAction::getBoxMin(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end) {
  auto_ptr<DeetooMessage> m (new DeetooMessage(start, end, true, _r, 0) );
  cout << "start: " << start << ", end: " << end << endl;
  cout << "in getBoxMin- 111111111111111" << endl;
  auto_ptr<DeetooNetwork> visited_net( m->visit(node, net) );
  cout << "in getBoxMin- 111111111111111" << endl;
  auto_ptr<NodeIterator> nit (visited_net->getNodeIterator() );
  cout << "in getBoxMin- 111111111111111" << endl;
  int box_min = BOX_M;
  int this_min;
  Box *min_box, *current_box;
  cout << "in getBoxMin- 111111111111111" << endl;
  ExactD2Node* this_node;
  pair<Box*, ExactD2Node*> result; 
  cout << "in getBoxMin- 111111111111111" << endl;
  while (nit->moveNext() ) {
    ExactD2Node* node = dynamic_cast<ExactD2Node*> (nit->current() );
    current_box = node->getBox();
    this_min = current_box->count();
    if (this_min < BOX_M) {
      box_min = this_min;
      min_box = current_box;
      this_node = node;
    }
  }
  cout << "in getBoxMin- 111111111111111" << endl;
  result = make_pair(min_box, this_node);
  return make_pair(box_min, result);
  //return result;
}
void ExactNodeJoinAction::split(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end, bool isColumn, bool isLeft) {
  
  auto_ptr<DeetooMessage> m (new DeetooMessage(start, end, isColumn, _r, 0) );
  auto_ptr<DeetooNetwork> visited_net( m->visit(node, net) );
  auto_ptr<NodeIterator> nit (visited_net->getNodeIterator() );
  Box *past = 0, *current;  
  int it_no = 0;
  while (nit->moveNext() ) {
    ExactD2Node* node = dynamic_cast<ExactD2Node*> (nit->current() );
    current = node->getBox();
    //pair<my_int, my_int> boundary = current->getBoundary();
    Box *new_box;
    //Let's add middle address to node's cols or rows
    my_int mid = current->getMiddle(isColumn);
    node->addColRow(mid, isColumn);

    if (it_no == 0 || !current->equalTo(past) ) {
      // first node accessed by split()
      // or box is not yet splitted by other node.
      // split this box anyway
      vector<my_int> new_boundary = current->getSplittedBoundary(isColumn);
      my_int start1 = new_boundary[0];
      my_int end1 = new_boundary[1];
      my_int start2 = new_boundary[2];
      my_int end2 = new_boundary[3];
      if(isLeft) { // left (col) or upper (row)
        current->update(start1, end1);
	new_box = new Box(start2, end2);
      }
      else {
        current->update(start2, end2);
	new_box = new Box(start1, end1);
      }
    }
    else {
      //box is already splitted
      //update and add this node to new box.
      my_int addr = node->getAddress(1);
      if (new_box->inBox(addr) ) {
        new_box->addNode(node,_cnet);
        //node->setBox(new_box);
        current->deleteNode(node);
      }
      else if (current->inBox(addr) ) {
        current->addNode(node,_cnet);
        //node->setBox(current);
        new_box->deleteNode(node);
      }
    }
    it_no++;
    past = current;
    //need to update cols, rows
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }

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
void ExactNodeJoinAction::getConnection(DeetooNetwork& net, ExactD2Node* me, bool cache)
{
  my_int addr = me->getAddress(cache);
  /**
  map<my_int, ExactD2Node*>::const_iterator ttt;
  for (ttt = net.node_map.begin(); ttt != net.node_map.end(); ttt++) {
    cout << ttt->first << ", ";
  }
  cout << endl;
  */
  // make connections: ring connection as well as shortcut connections
  if (net.getNodeSize() == 0) {
    // *me* is the only node in this network
    // no connection is needed.
  }
  // There is only one node in the network. Make connection with it.
  else if ( net.getNodeSize() == 1) {
    ExactD2Node* neighbor = dynamic_cast<ExactD2Node*> (net.node_map.begin()->second);
    if(neighbor != 0) {
      net.add(Edge(me, neighbor));
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
      //cout << "old_edge: " << no0->getAddress(cache)
	//    << " : " << no1->getAddress(cache) << endl;
      net.remove(old_edge);
    }
    ExactD2Node* shortcut = dynamic_cast<ExactD2Node*> (net.returnShortcutNode(me, net.node_map,true) );
    if(!(net.getEdge(me, neighbor0)) && !(net.getEdge(neighbor0, me))) {
      net.add(Edge(me, neighbor0));
    }
    if(!(net.getEdge(me, neighbor1)) && !(net.getEdge(neighbor1, me))) {
      net.add(Edge(me, neighbor1));
    }
    if(!(net.getEdge(me, shortcut)) && !(net.getEdge(shortcut, me))) {
      net.add(Edge(me, shortcut));
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

    //make sure ring is closed.
    ExactD2Node* front = dynamic_cast<ExactD2Node*> (net.node_map.begin()->second);
    ExactD2Node* back = dynamic_cast<ExactD2Node*> ((net.node_map.rbegin())->second);
    net.add(Edge(front,back));
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
  /*
  double guess = _net.guessNetSizeLog(node,1,1);
  //double guess = _net.getNodeSize();
  //cout << "c_addr: " << node->getAddress(1) << ", sq_alpha: " << _sq_alpha << ", guessNetSize: " << guess << endl;
  double cqsize = (double) (((AMAX) / (double)sqrt(guess ) ) * _sq_alpha);
  //cout << "cache::cqsize: " << cqsize << "\t addr: " << node->getAddress(1) << endl;
  std::pair<my_int, my_int> range = _net.getRange(cqsize);
  my_int rg_start = range.first;
  my_int rg_end = range.second;
  //cout << "rg_start: "<< rg_start << ", rg_end: " << rg_end << ", diff: " << rg_end-rg_start << endl;
  //_so.start = rg_start;
  //_so.end = rg_end;
  //node->insertObject(_so);
  */
  Box* this_box = node->getBox();
  pair<my_int,my_int> range = this_box->getRange(1);
  my_int rg_start = range.first * AMAX;
  my_int rg_end = (range.second * AMAX) + (AMAX -1);
  auto_ptr<DeetooMessage> cache_m(new DeetooMessage(rg_start, rg_end, true, _r, 0.0) );
  auto_ptr<DeetooNetwork> tmp_net (cache_m->visit(node, _net));
  auto_ptr<NodeIterator> ni (tmp_net->getNodeIterator() );
  while (ni->moveNext() ) {
    ExactD2Node* inNode = dynamic_cast<ExactD2Node*> (ni->current() );
    //cout << "----------cacheaction: " << endl;
    //cout << "addr: " << inNode->getAddress(1) << "item: " << _so.content << endl;
    //cout << "size before insertion: " << (inNode->getObject()).size() << endl;
    inNode->insertObject(_so, rg_start, rg_end);
    //cout << "size after insertion: " << (inNode->getObject()).size() << endl;
  }
  /*
  node->stabilize(cqsize);
  */
#ifdef DEBUG
  std::cout << _sched.getCurrentTime() << "\t"
	    << "caching\t"
            << _net.getNodeSize() << "\t"
            << _net.getEdgeSize() << "\t"
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
  //cout << "QueryAction+++++++++++++++++++++++++++++++++++++++" << endl;
  //schedule a time to query object to nodes in the range:
  //cout << "queryaction start here" << endl;
  UniformNodeSelector u_node(_r);
  _ns.selectFrom(&_net);
  ExactD2Node* node = dynamic_cast<ExactD2Node*> (_ns.select() );
  /*
  double guess = _net.guessNetSizeLog(node,0,1);
  //cout << "querying: guess: " << guess << endl;
  //double guess = _net.getNodeSize();
  //cout << "q_addr: " << node->getAddress(0) << ", sq_alpha: " << _sq_alpha << ", guessNetSize: " << guess << endl;
  //double cqsize = (double) (((WMAX ) / (double)sqrt(_net.guessNetSizeLog(node,0) ) ) * _sq_alpha);
  double cqsize = (double) (((AMAX ) / (double)sqrt(guess ) ) * _sq_alpha);
  //cout << "query::cqsize: " << cqsize << "\t addr: " << node->getAddress(0) << endl;
  std::pair<my_int, my_int> range = _net.getRange(cqsize);
  my_int rg_start = range.first, rg_end = range.second;
  //cout << "rg_start: "<< rg_start << ", rg_end: " << rg_end << ", diff: " << rg_end-rg_start << endl;

  */
  Box* this_box = node->getBox();
  pair<my_int, my_int> range = this_box->getRange(0);
  my_int rg_start = range.first;
  my_int rg_end = range.second;
  auto_ptr<DeetooMessage> query_m (new DeetooMessage(rg_start, rg_end,false, _r, 0.0) );
  auto_ptr<DeetooNetwork> tmp_net (query_m->visit(node, _net));
  no_msg = tmp_net->getNodeSize();
  int q_in_depth = tmp_net->getDistance(query_m->init_node);
  depth = q_in_depth + query_m->out_edge_count;
  sum_hits = 0;
  auto_ptr<NodeIterator> ni (tmp_net->getNodeIterator() );
  while (ni->moveNext() ) {
    ExactD2Node* inNode = dynamic_cast<ExactD2Node*> (ni->current() );

    //if (_so == 0) { cout << "null" << endl; }
    //else { cout << "not null" << endl; }
    //cout << "# objects: " << inNode->getObject().size() << endl;
    sum_hits += inNode->searchObject(_so);
  } 
  double hit_rate = (double)sum_hits / (double)no_msg;
#ifdef DEBUG
  std::cout << _sched.getCurrentTime() << "\t"
	    << "querying\t"
            << _net.getNodeSize() << "\t"
            << _net.getEdgeSize() << "\t"
	    << sum_hits << "\t"
	    << no_msg << "\t"
	    << hit_rate << "\t"
	    << q_in_depth << "\t"
            << depth << "\t"
	    << std::endl;
#endif
}


