/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2005  University of California
Copyright (C) 2005  P. Oscar Boykin <boykin@pobox.com>, University of Florida

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

#ifndef starsky__contentnetwork
#define starsky__contentnetwork

#include "network.h"
#include "node.h"
#include "contentnode.h"
#include "message.h"
#include <map>

namespace Starsky {

 /**
  * Created from an existing network, this class keeps track
  * of content associated with nodes.  This allows modeling of
  * queries and caching on P2P systems.  Content is modeled as a node.
  * This class actually models the content subgraph and the node partial subgraph
  * separately.
  * @see Starsky::ContentNode
  */
	
class ContentNetwork : public Network {

    public:
        ContentNetwork();
	/**
	 * @param aNet the network topology to use for the nodes
	 */
        ContentNetwork(Network& aNet);
	/**
	 * delete all the content nodes.
	 */
	void deleteContent();
        /**
	 * @return set of all content that has been inserted
	 */
	const std::set<ContentNode*>& getContent() const;
	/**
	 * @param c the content to get all nodes which hold it.
	 * @return the set of all nodes which hold content c
	 */
	const Network::NodePSet& getNodesHoldingContent(ContentNode* c) const;
	/**
	 * @param node the node that inserts the content.
	 * @param content the content to be inserted
	 * @param amessage the message which starts at node, and all visited nodes get content
	 */
	virtual void insertContent(Node* node, ContentNode* content, Message& amessage);
	/**
	 * @param node the node to start the query at
	 * @param content the content we are searching for
	 * @param amessage the message to carry the query
	 * @return set of nodes which are reached by the message and have the content
	 */
	virtual Network::NodePSet queryForContent(Node* node,
			                  ContentNode* content,
					  Message& amessage);


    protected:
	/**
	 * Reference to the network with the nodes in it
	 */
	Network& _my_net;
	/**
	 * holds the set of all content held by each node
	 */
	std::map< Node*, std::set<ContentNode*> > _content_map;
	/**
	 * holds the nodes which hold a given content:
	 */
	std::map<ContentNode*, Network::NodePSet> _content_to_nodes;
	/**
	 * holds the set of all content
	 */
	std::set<ContentNode*> _content_set;
};
	
}

#endif
