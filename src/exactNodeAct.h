/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2008  Taewoong Choi <twchoi@ufl.edu>, University of Florida

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

#ifndef starsky__exactnodeact_h
#define starsky__exactnodeact_h

#include "event.h"
#include "netmodeler.h"
#include "exactd2node.h"
#include "edge.h"
#include "box.h"
#include <map>
#include <iostream>
#include <cmath>

#ifdef INT64
  typedef unsigned long long my_int;
  #define WMAX 18446744073709551615LL
#else
  typedef unsigned long my_int;
  #define WMAX 4294967295L
#endif

namespace Starsky {
  // action for node leaves
  class ExactNodeLeaveAction : public Action {
    public:
      ExactNodeLeaveAction(EventScheduler& sched, DeetooNetwork& cn, DeetooNetwork& qn, ExactD2Node* me); 
      void Execute();
    protected:
      EventScheduler& _sched;
      DeetooNetwork& _cnet;
      DeetooNetwork& _qnet;
      ExactD2Node* _me;
  };

  // action for node joins 
  class ExactNodeJoinAction : public Action {
    public:
      ExactNodeJoinAction(EventScheduler& sched, Random& r, DeetooNetwork& cn, DeetooNetwork& qn);
      void Execute();
      /**
       * make connections to 2 direct neighbors and 1 shortcut neighbor.
       * @param net network which a node joins
       * @param me joining node
       * @param cache determine if this network is for cache or for query.
       */
      void getConnection(DeetooNetwork& net, ExactD2Node* me, bool cache);
      /**
       * copy objects from neighbors
       * @param me joing node
       * @param nei neighbor node
       * @return returns number of copied objects
       */
      int copyObjects(ExactD2Node* me, ExactD2Node* nei);
      int stabilization_msgs;
    protected:
      EventScheduler& _sched;
      Random& _r;
      DeetooNetwork& _cnet;
      DeetooNetwork& _qnet;
      double _sq_alpha;
      pair<int, pair<Box*, ExactD2Node*> > getBoxMin(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end);
      void printNetworkInfo(Network& net, bool bo);
      /*
       *@param isLoU true if a node is left or upper
       */
      void split(DeetooNetwork& net, ExactD2Node* node, my_int start, my_int end, bool isColumn, bool isLeft);
  };

  // action for caching objects in the network
  class ExactCacheAction : public Action {
    public:
      ExactCacheAction(EventScheduler& sched, Random& r, INodeSelector& ns, DeetooNetwork& net, string& so);
      void Execute();
    protected:
      EventScheduler& _sched;
      Random& _r;
      DeetooNetwork& _net;
      INodeSelector& _ns;
      string _so;
  };
  // action for querying objects
  class ExactQueryAction : public Action {
    public:
      //bool hit = false;
      int sum_hits;
      int no_msg;
      int depth;
      ExactQueryAction(EventScheduler& sched, Random& r, INodeSelector& ns, DeetooNetwork& net, string so);
      void Execute();
    protected:
      EventScheduler& _sched;
      Random& _r;
      DeetooNetwork& _net;
      INodeSelector& _ns;
      string _so;
  };
}
#endif


