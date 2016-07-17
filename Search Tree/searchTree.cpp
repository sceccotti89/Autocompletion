/*
 * searchTree.cpp
 *
 *  Created on: 01 ott 2015
 *      Author: stefano
 */

#include "searchTree.hpp"

using namespace search_tree;

static inline void retrieveData( const string& P, vector<string>& list, searchTree* node, int level )
{
	//string* word = node->getWord();
	if(!node->isEmpty())
		list.push_back( P.substr( 0, level ) );

	#if VERBOSE
	if(!node->isEmpty())
		cout << "INSERTED: " + P.substr( 0, level ) + ", AT LEVEL: " << level << endl;
	#endif

	map<char, searchTree*> nodes = node->getMap();
	for(auto& item : nodes) // auto = pair<const char, searchTree*>
		retrieveData( P, list, item.second, level + 1 );
}

void searchTree::strongPrefix( const string& P, const unsigned int length, vector<string>& list )
{
	searchTree* node = this;
	unsigned char index;
	unsigned int level;

	for(level = 0; level < length; level++){
		index = P[level];

		try{
			node = node->getMap().at( index );
		}catch( out_of_range& e ){
			break;
		}
	}

	if(level > 0) // retrieve data only if there is at least one matching character
		retrieveData( P, list, node, 0 );
}

void searchTree::weakPrefix( const string& P, const unsigned int length, vector<string>& list )
{
	searchTree* node = this;
	unsigned char index;
	unsigned int level;

	for(level = 0; level < length; level++){
		index = P[level];

		try{
			node = node->getMap().at( index );
		}catch( out_of_range& e ){
			break;
		}
	}

	#if VERBOSE
	cout << "CURRENT LEVEL: " << level << " - " + P.substr( 0, level ) << endl;
	#endif

	if(level == length) // retrieve data only if all the characters match the pattern
		retrieveData( P, list, node, level );
}

void searchTree::deleteNode( const string& P, const unsigned int length ) // FIXME
{
	searchTree* node = this;
	unsigned char index;
	unsigned int level, remove_from = 0;
	searchTree* from_node = nullptr;

	for(level = 0; level < length; level++){
		if(level > 0){
			if(from_node == nullptr && node->nodes.size() == 1){
				remove_from = level;
				from_node = node;
			}
			else
				from_node = nullptr;
		}

		index = P[level];

		try{
			node = node->getMap().at( index );
		}catch( out_of_range& e ){
			#if VERBOSE
			cout << "pattern: \"" + P + "\" not found" << endl;
			#endif

			return;
		}
	}

	if(node->isEmpty()){
		#if VERBOSE
		cout << "pattern: \"" + P + "\" not found" << endl;
		#endif

		return;
	}

	if(from_node != nullptr){
		// match founded => remove the node until remove_node
		for(unsigned int level = remove_from; level < length; level++){
			index = P[level];
			node = from_node->getMap().at( index );
			free( from_node );
			from_node = node;
		}
	}
}

/*void patriciaTrie::weakPrefix( const string& P, const unsigned int length, vector<string>& list )
{

}

void patriciaTrie::deleteNode( const string& P, const unsigned int length )
{

}*/
