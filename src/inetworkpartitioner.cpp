#include "inetworkpartitioner.h"

using namespace std;
using namespace Starsky;

void INetworkPartitioner::deletePartition(set<Network*>* part)
{
  set<Network*>::iterator it;
  for(it = part->begin(); it != part->end(); it++)
  {
    //Delete the Network:
    delete *it;
  }
  //Delete the Set:
  delete part;
}

double INetworkPartitioner::modularityOf(set<Network*>& partition,
		                         const Network& orig)
{

  //Now net_vec indexes the communities
  map<Node*, Network*> node_community;
  set<Network*>::const_iterator netit;
  for(netit = partition.begin(); netit != partition.end(); netit++)
  {
    Network::NodePSet::const_iterator nit;
    const Network::NodePSet& nodes = (*netit)->getNodes();
    for(nit = nodes.begin(); nit != nodes.end(); nit++)
    {
      node_community[ *nit ] = *netit;
    }
  }
  
  map<Network*, map<Network*, double> > e_ij;
  const Network::EdgeSet& edge_set = orig.getEdges();
  Network::EdgeSet::const_iterator e_it;
  Network *com1, *com2;
  double e_total = 0.0;
  for(e_it = edge_set.begin();
      e_it != edge_set.end();
      e_it++) {
    com1 = node_community[ (*e_it)->first ];
    com2 = node_community[ (*e_it)->second ];
    e_ij[com1][com2] += 1.0;
    e_ij[com2][com1] += 1.0;
    e_total += 2.0;
  }
  //Normalize e_ij:
  set<Network*>::const_iterator netit2;
  for(netit = partition.begin(); netit != partition.end(); netit++)
  {
    Network* ni = *netit;
    for(netit2 = partition.begin(); netit2 != partition.end(); netit2++)
    {
      Network* nj = *netit2;
      e_ij[ni][nj] /= e_total;
    }
  }
  //Make a_i;
  map<Network*, double> a_i;
  for(netit = partition.begin(); netit != partition.end(); netit++)
  {
    Network* ni = *netit;
    a_i[ni] = 0.0;
    for(netit2 = partition.begin(); netit2 != partition.end(); netit2++)
    {
      Network* nj = *netit2;
      a_i[ni] += e_ij[ni][nj];
    }
  }
  //We don't neccesarily start at Q=0;
  double q = 0.0;
  //for(int i = 0; i < e_ij.size(); i++) {
  for(netit = partition.begin(); netit != partition.end(); netit++)
  {
    Network* i = *netit;
    q += e_ij[i][i] - a_i[i] * a_i[i];
  }
  
  return q;
	
}
