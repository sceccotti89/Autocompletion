/*
 * searchTree.h
 *
 *  Created on: 01 ott 2015
 *      Author: stefano
 */

#ifndef SEARCHTREE_HPP_
#define SEARCHTREE_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <unistd.h>

#include "objectPool.hpp"
#include "file_manager.hpp"
#include "node_manager.hpp"
#include "../Hashing/min_perfect_hashing.hpp"

using namespace std;

namespace search_tree
{
	#pragma pack( 1 )
	class searchTree
	{
		private:
			/* map of attached nodes */
			map<char, searchTree*> nodes;
			/* used to check if the node points to a dictionary string */
			unsigned int is_empty : 1;

		public:
			searchTree() : is_empty( 1 ) {}

			~searchTree(){}

			/***/
			void weakPrefix( const string& P, const unsigned int length, vector<string>& list );

			/** returns all the strings sharing the longest common prefix with P */
			void strongPrefix( const string& P, const unsigned int length, vector<string>& list );

			/** adds a node to the tree */
			inline void addNode( const string& word, const unsigned int length )
			{
				uint8_t index;
				searchTree* node = this, *tmp_node;

				for(unsigned int level = 0; level < length; level++){
					index = word[level];

					try{
						node = node->nodes.at( index );
					}
					catch( out_of_range& e ){
						node->nodes.insert( { index, tmp_node = new searchTree() } );
						node = tmp_node;
					}
				}

				node->is_empty = 0;
			}

			/***/
			void deleteNode( const string& P, const unsigned int length );

			/***/
			inline unsigned int isEmpty(){ return is_empty; }

			/***/
			inline map<char, searchTree*> getMap(){ return nodes; }
	};

	/* implemented with unordered map */
	class patriciaTrie
	{
		private:
			#pragma pack( 1 )
			class node_t
			{
				public:
					/* map of attached nodes */
					map<char, uint64_t>* nodes;
					/* position of the word on disk */
					int64_t file_offset;
					/* offset in the string respect to the initial position */
					uint64_t offset;
					/* position in the scores array */
					int64_t index;

					node_t() : file_offset( -1 ), offset( 0 ), index( -1 )
					{
						nodes = nullptr;
					}

					~node_t()
					{
						//delete nodes;
					}

					/***/
					inline void init()
					{
						offset = 0;
						file_offset = -1;
						index = -1;
						nodes = nullptr;
					}

					/***/
					inline uint64_t get( char position )
					{
						return nodes->at( position );
					}

					/***/
					inline uint64_t at( char index )
					{
						uint8_t i = 0;
						for(pair<const char, uint64_t>& item : *nodes){
							if(i == index)
								return item.second;
							else
								i++;
						}

						return -1;
					}

					/***/
					inline void print_all( unsigned int level, objectPool<node_t>* pool, file_manager& file_man )
					{
						node_t* tmp_node;
						for(pair<const char, uint64_t>& item : *nodes){
							tmp_node = pool->get( item.second );
							cout << "FROM: " << level << ", SIZE: " << nodes->size() << ", ";
							if(tmp_node->file_offset >= 0)
								cout << "WORD: " << file_man.read( tmp_node->file_offset ) << ", ";
							cout << "LEVEL: " << (level + 1) << ", CHAR: " << item.first << ", OFFSET: " << tmp_node->offset << endl;

							tmp_node->print_all( level + 1, pool, file_man );
						}
					}

					/***/
					inline void retrieveData( vector<string>& list, objectPool<node_t>* obj_pool, file_manager& file_man )
					{
						if(file_offset >= 0)
							list.push_back( file_man.read( file_offset ) );

						if(nodes != nullptr){
							for(pair<const char, uint64_t>& item : *nodes)
								obj_pool->get( item.second )->retrieveData( list, obj_pool, file_man );
						}
					}
			};

			/* root of the trie */
			node_t root;
			/* roots used for parallel computation */
			//patriciaTrie** trie_t = nullptr;
			/* size of the object pool */
			static const constexpr uint64_t POOL_SIZE = 1 << 19;
			/* pool of nodes */
			objectPool<node_t>* obj_pool = nullptr;
			/* manager used to write and read from disk */
			file_manager* f_manager;
			/* index offset */
			uint64_t index_offset = 0;
			/* threshold to decides if the computation is in parallel */
			static const constexpr uint32_t THRESHOLD = 1 << 22;
			/* decides if the computation is done in parallel */
			bool parallel = false;
			/* number of insertions */
			uint64_t size = 0;
			/* starting string used in the parallel version */
			string _start;

			/***/
			inline void build( const vector<string>& input, const uint64_t size )
			{
				for(uint64_t i = 0; i < size; i++){
					addNode( input[i], input[i].size(), i );
					cout << "ADDED: " << input[i] << endl;
					//TODO scores.push_back( std::rand() % 1000 );
				}
			}

			/** merge two patricia trie */
			inline void merge( const patriciaTrie* other, const string _start )
			{
				//map<char, uint64_t>* _nodes = other.root.nodes;
				//cout << "NODES: " << (_nodes == nullptr) << endl;
				addNode( _start, _start.length(), 0, other->root.nodes );
				f_manager->merge( other->f_manager );
				obj_pool->merge( other->obj_pool );

				//node_t *node = &root;
				//cout << "CERCO IL NODO MISMATCHING" << endl;
				//get_mismatching_node( &node, _start, _start.length() );
				//if(node->nodes == nullptr) node->nodes = new map<char, uint64_t>();
				//node->nodes->insert( _nodes->begin(), _nodes->end() );

				/*if(root.nodes == nullptr)
					root.nodes = new map<char, uint64_t>();

				for(pair<const char, uint64_t>& item : *_nodes)
					root.nodes->insert( item );*/
			}

			/***/
			explicit patriciaTrie( const uint64_t offset, const int16_t index, const string s_string )
			{
				index_offset = offset;
				f_manager = new file_manager( "strings" + to_string( index ) + ".txt" );
				obj_pool = new objectPool<node_t>( POOL_SIZE );
				_start = s_string;
				parallel = true;
			}

		public:
			/** Build a patricia trie starting from an ordered or unordered input.
			 *  Use addNode or deleteNode to insert or delete a node.
			*/
			explicit patriciaTrie()
			{
				obj_pool = new objectPool<node_t>( POOL_SIZE );
				f_manager = new file_manager();
			}

			/** Build a patricia trie starting from an ordered input.
			 *  This constructor allows you to build in a parallel way the entire trie,
			 *  saving a lot of time at a cost of more space consuming, due to the threads activation.
			 *
			 * @param input  - list of input strings
			 * @param size	 - input size
			*/
			explicit inline patriciaTrie( const vector<string>& input, const uint64_t size )
			{
				obj_pool = new objectPool<node_t>( (size << 1) + 1 );
				f_manager = new file_manager();

				uint64_t numCPU = sysconf( _SC_NPROCESSORS_ONLN );
				if(size < numCPU){
					build( input, size );
					return;
				}

				cout << "PROC: " << numCPU << endl;

				uint64_t n = size / numCPU;
				thread** pool_t = new thread*[numCPU];
				patriciaTrie** trie_t = new patriciaTrie*[numCPU];
				uint64_t i, from, to;
				cout << "LANCIO I THREAD..." << endl;
				for(i = 0; i < numCPU - 1; i++){
					from = i * n;
					to = (i + 1) * n;
					trie_t[i] = new patriciaTrie( from, i, input[from] );
					pool_t[i] = new thread( [&]( patriciaTrie* trie ){
						for(uint64_t i = from; i < to; i++){
							trie->addNode( input[i], input[i].length(), i );
							cout << "ADDED: " << input[i] << endl;
						}
					}, trie_t[i] );
				}

				from = size - n;
				trie_t[i] = new patriciaTrie( from, i, input[from] );
				pool_t[i] = new thread( [&]( patriciaTrie* trie ){
					for(uint64_t i = from; i < size; i++){
						trie->addNode( input[i], input[i].length(), i );
						cout << "ADDED: " << input[i] << endl;
					}
				}, trie_t[i] );
				cout << "THREAD LANCIATI" << endl;

				// wait for threads termination
				for(uint64_t i = 0; i < numCPU; i++){
					pool_t[i]->join();
					delete pool_t[i];
				}

				cout << "THREAD TERMINATI" << endl;

				// TODO ottimizzare eseguendo una reduce
				// merge the parallel tries
				cout << "UNISCO I TRIE..." << endl;
				for(uint64_t i = 0; i < numCPU; i++){
					cout << "STRING: " + trie_t[i]->_start << endl;
					merge( trie_t[i], trie_t[i]->_start );
					delete trie_t[i];
				}

				delete[] pool_t;
				delete[] trie_t;
			}

			~patriciaTrie()
			{
				if(!parallel){
					delete obj_pool;
					delete f_manager;
				}
			}

			/***/
			inline int8_t get_index_mismatch( const string& a, const uint64_t a_size,
											  const string& b, const uint64_t b_size,
											  uint64_t* mismatch )
			{
				uint64_t i, l = (a_size < b_size) ? a_size : b_size;
				for(i = 0; i < l; i++){
					if(a[i] != b[i])
						break;
				}

				*mismatch = i;

				// checks if the two strings are the same
				if(a_size == b_size && i == l)
					return -1;
				else
					return 0;
			}

			/***/
			inline string get_string_on_leaf( node_t* node )
			{
				if(node->file_offset == -1){
					// goes down until we found a leaf
					node = obj_pool->get( node->nodes->begin()->second );
					for(; node->file_offset == -1; node = obj_pool->get( node->nodes->begin()->second ));
				}

				return f_manager->read( node->file_offset, parallel );
			}

			/** add a child to the current node */
			inline void insert_node( node_t* node, const int64_t _index, const char c,
									 const string& word, const unsigned int offset,
									 const uint64_t score_index, const map<char, uint64_t>* _nodes = nullptr )
			{
				uint64_t _idx = 0;
				bool resized = 0;
				node_t* tmp_node = obj_pool->allocate( 1, &resized, &_idx, parallel );
				if(resized && _index >= 0)
					node = obj_pool->get( _index );

				tmp_node->offset = offset;

				if(node->nodes == nullptr)
					node->nodes = new map<char, uint64_t>();

				node->nodes->insert( { c, _idx } );

				if(_nodes != nullptr){
					tmp_node->nodes = new map<char, uint64_t>();
					tmp_node->nodes->insert( _nodes->begin(), _nodes->end() );
				}
				else{
					tmp_node->index = score_index;
					tmp_node->file_offset = f_manager->write( word, word.length(), parallel );
				}
			}

			/** scan the trie until it doesn't found a branch */
			inline unsigned int get_mismatching_node( node_t** node, const string& word, const unsigned int length )
			{
				unsigned int offset;
				node_t* tmp_node = *node;

				for(offset = 0; offset < length; offset += tmp_node->offset){
					if(tmp_node->nodes == nullptr)
						break;

					try{
						tmp_node = obj_pool->get( tmp_node->nodes->at( word[offset] ) );
					}
					catch( out_of_range& e ){
						break;
					}
				}

				*node = tmp_node;

				return offset;
			}

			/***/
			inline void print_all()
			{
				root.print_all( 0, obj_pool, *f_manager );
			}

			/** adds a node to the trie
			 *
			 * @param word   - word to add
			 * @param length - length of the word
			*/
			inline void addNode( const string& word, const unsigned int length,
								 const uint64_t index, const map<char, uint64_t>* _nodes = nullptr )
			{
				node_t* node = &root, *tmp_node;
				uint64_t offset, mismatch;

				offset = get_mismatching_node( &node, word, length );

				// avoid the scan of the trie, since there is only the root
				if(offset == 0){
					insert_node( node, -1, word[0], word, length, index, _nodes );
					return;
				}

				string s = get_string_on_leaf( node );

				// if the two strings are the same it will not be added
				if(get_index_mismatch( word, length, s, s.length(), &mismatch ) == -1)
					return;

				node = &root;
				int64_t _index = -1;
				bool resized = 0;
				for(offset = 0; ; offset = offset + node->offset){
					if(offset == mismatch){
						// if the value is equal to the offset of a node, just add the node and exit
						if(offset == length)
							node->file_offset = f_manager->write( word, word.length(), parallel );
						else
							insert_node( node, _index, word[offset], word, length - offset, index, _nodes );

						break;
					}
					else{
						if(offset > mismatch){
							unsigned int diff = offset - mismatch;
							node->offset = node->offset - diff;
							uint64_t idx = 0;

							// creates a new node copying the children nodes of the mismatching one
							tmp_node = obj_pool->allocate( 1, &resized, &idx, parallel );
							if(resized){
								resized = 0;
								node = obj_pool->get( _index );
							}

							if(node->nodes != nullptr) // assign the values from the father to the child node
								tmp_node->nodes = node->nodes;
							node->nodes = new map<char, uint64_t>();

							// add the node to the current one
							node->nodes->insert( { s[mismatch], idx } );
							tmp_node->offset = diff;

							tmp_node->file_offset = node->file_offset;
							node->file_offset = -1;
							tmp_node->index = node->index;

							// add the new word
							insert_node( node, _index, word[mismatch], word, length - mismatch, index, _nodes );

							break;
						}
						else // here the exception cannot be raised
							node = obj_pool->get( _index = node->nodes->at( word[offset] ) );
					}
				}
			}

			/***/
			inline void strongPrefix( const string& P, const unsigned int length, vector<string>& list )
			{
				/*
				thread** pool_t = nullptr;
				vector<string>* lists = nullptr;

				if(trie_t.size() > 1){ // parallel execution
					uint64_t numCPU = sysconf( _SC_NPROCESSORS_ONLN ) - 1;

					lists = new vector<string> list[numCPU];
					pool_t = new thread*[numCPU];
					cout << "LANCIO I THREAD PER IL PREFISSO..." << endl;
					for(i = 0; i < numCPU; i++){
						pool_t[i] = new thread( [&]( patriciaTrie* trie ){
							for(uint64_t i = from; i < to; i++){
								trie->strongPrefix( P, length, lists[i] );
								sort( lists[i].begin(), lists[i].end() );
							}
						}, trie_t[i] );
					}
				}
				*/

				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = get_mismatching_node( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );

				node = &root;
				for(offset = 0; offset < length; offset += node->offset){
					if(offset >= mismatch){
						cout << "PREFIX SHARED: " + P.substr( 0, offset ) << endl;
						node->retrieveData( list, obj_pool, *f_manager );
						return;
					}
					else // here the exception cannot be raised
						node = obj_pool->get( node->nodes->at( P[offset] ) );
						//node = obj_pool->get( node->get( P[offset] ) );
				}

				node->retrieveData( list, obj_pool, *f_manager );

				/*
				if(trie_t.size() > 1){
					// wait for the threads termination
					uint64_t numCPU = pool_t.size();
					for(uint64_t i = 0; i < numCPU; i++){
						pool_t[i]->join();
						delete pool_t[i];
					}

					delete[] pool_t;

					// TODO ottimizzabile tramite reduce
					// merge the partial results using the binary search
					for(uint64_t i = 0; i < numCPU; i++)
						std::merge( list.begin(), list.end(), lists[i].begin(), lists[i].end(), list.begin() );
						//list.insert( list.end(), lists[i].begin(), lists[i].end() );
				}
				*/
			}

			/***/
			inline void weakPrefix( const string& P, const unsigned int length, vector<string>& list )
			{
				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = get_mismatching_node( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );
				if(mismatch < length){
					cout << "prefix " + P + " not found" << endl;
					return;
				}

				node = &root;
				for(offset = 0; offset < length; offset += node->offset)
					//node = obj_pool->get( node->nodes->at( P[offset] ) );
					node = obj_pool->get( node->get( P[offset] ) );

				node->retrieveData( list, obj_pool, *f_manager );
			}

			/***/
			void finish()
			{
				obj_pool->finish();
				f_manager->finish();
			}

			/***/
			void deleteNode( const string& P, const unsigned int length );

			/***/
			void setDFOUDS()
			{
				// TODO
			}
	};

	/* implemented with list and binary search */
	#pragma pack( 1 )
	class patriciaTrie2
	{
		private:
			class node_t
			{
				public:
					/* array of attached nodes */
					pair<char, uint64_t>* nodes;
					/* number of nodes */
					uint8_t size;
					/* position of the word on disk */
					int64_t file_offset;
					/* offset in the string respect to the initial position */
					uint64_t offset;
					/* position in the scores array */
					int64_t index;

					node_t() :
						nodes( nullptr ),
						size( 0 ),
						file_offset( -1 ),
						offset( 0 ),
						index( -1 )
					{

					}

					~node_t(){}

					/***/
					inline void init()
					{
						nodes = nullptr;
						size = 0;
						offset = 0;
						file_offset = -1;
						index = -1;
					}

					/***/
					/*inline uint64_t get( char position )
					{
						return nodes[position].second;
					}*/

					/***/
					/*inline uint64_t at( char index )
					{
						return nodes[index].second;
					}*/

					/***/
					inline void remove( objectPool<node_t>* obj_pool )
					{
						if(nodes != nullptr){
							for(uint8_t i = 0; i < size; i++)
								obj_pool->get( nodes[i].second )->remove( obj_pool );

							delete[] nodes;
						}

						this->~node_t();
					}

					/***/
					inline void print_all( const unsigned int level, objectPool<node_t>* obj_pool, file_manager& file_man )
					{
						if(nodes != nullptr){
							for(uint8_t i = 0; i < size; i++){
								node_t* node = obj_pool->get( nodes[i].second );
								cout << "FROM: " << level << ", SIZE: " << size << ", ";
								if(node->file_offset >= 0)
									cout << "WORD: " << file_man.read( node->file_offset ) << ", ";
								cout << "LEVEL: " << (level + 1) << ", CHAR: " << nodes[i].first << ", OFFSET: " << node->offset << endl;

								node->print_all( level + 1, obj_pool, file_man );
							}
						}
					}

					/***/
					inline void retrieveData( vector<string>& list, objectPool<node_t>* obj_pool, file_manager& file_man )
					{
						if(file_offset >= 0)
							list.push_back( file_man.read( file_offset ) );

						if(nodes != nullptr){
							for(uint8_t i = 0; i < size; i++)
								obj_pool->get( nodes[i].second )->retrieveData( list, obj_pool, file_man );
						}
					}
			};

			/* root of the trie */
			node_t root;
			/* pool of nodes */
			objectPool<node_t>* obj_pool = nullptr;
			/* manager for writes and reads from disk */
			file_manager f_manager;

		public:
			patriciaTrie2()
			{
				obj_pool = new objectPool<node_t>( 1 << 19 );
			}

			~patriciaTrie2()
			{
				root.remove( obj_pool );
				obj_pool->~objectPool();
			}

			/***/
			inline int8_t get_index_mismatch( const string& a, const uint64_t a_size,
											  const string& b, const uint64_t b_size,
											  uint64_t* mismatch )
			{
				uint64_t i, l = (a_size < b_size) ? a_size : b_size;
				for(i = 0; i < l; i++){
					if(a[i] != b[i])
						break;
				}

				*mismatch = i;

				// checks if the two strings are the same
				if(a_size == b_size && i == l)
					return -1;
				else
					return 0;
			}

			/***/
			inline string get_string_on_leaf( node_t* node )
			{
				if(node->file_offset == -1){
					// goes down until we found a leaf
					node = obj_pool->get( node->nodes[0].second );
					for(; node->file_offset == -1; node = obj_pool->get( node->nodes[0].second ));
				}

				return f_manager.read( node->file_offset );
			}

			/***/
			inline int16_t binary_search( const node_t* node, const uint16_t key, const bool get_position = false )
			{
				int16_t min = 0, mid = 0, max = node->size - 1;
				int16_t current = 0;

				while(min <= max){
					mid = (max + min) / 2;
					current = node->nodes[mid].first;

					if(current == key)
						return mid;
					else if(current < key)
						min = mid + 1;
					else
						max = mid - 1;
				}

				if(get_position){
					if(current < key) return min;
					else return max;
				}

				return -1;
			}

			/** add a child to the current node */
			inline void insert_node( node_t* node, const int64_t _index, const char c,
									 const string& word, const uint32_t offset, const uint64_t index )
			{
				uint64_t idx;
				bool resized = 0;
				node_t* _node = obj_pool->allocate( 1, &resized, &idx );

				if(resized && _index >= 0)
					node = obj_pool->get( _index );

				if(node->nodes == nullptr){
					//node->nodes = new pair<char, uint64_t>[node->size = 1];
					node->nodes = (pair<char, uint64_t>*) malloc( (node->size = 1) * sizeof( pair<char, uint64_t> ) );
					node->nodes[0] = { c, idx };
				}
				else{
					// gets the position where the item have to be inserted
					//pair<char, uint64_t>* nodes = node->nodes;
					//uint16_t position = lower_bound( nodes, nodes + node->size, c ) - nodes;
					int16_t position = binary_search( node, c, true );
					if(position < 0) position = 0;

					// creates a new array with 1 more cell
					uint8_t size = node->size++;
					pair<char, uint64_t>* _nodes = (pair<char, uint64_t>*) malloc( (size + 1) * sizeof( pair<char, uint64_t> ) );
					//pair<char, uint64_t>* _nodes = new pair<char, uint64_t>[size + 1];

					// copy phase
					memcpy( _nodes, node->nodes, position * sizeof( pair<char, uint64_t> ) );
					_nodes[position] = { c, idx };
					memcpy( _nodes + position + 1, node->nodes + position, (size - position) * sizeof( pair<char, uint64_t> ) );

					//delete[] node->nodes;
					free( node->nodes );
					node->nodes = _nodes;
				}

				_node->offset = offset;
				_node->index = index;
				_node->file_offset = f_manager.write( word, word.length() );
			}

			/** scans the trie until the first mismatching branch */
			inline uint32_t scan_trie( node_t** node, const string& word, const uint32_t length )
			{
				uint32_t offset;
				int16_t index;
				node_t* tmp_node = *node;

				for(offset = 0; offset < length; offset += tmp_node->offset){
					if(tmp_node->nodes == nullptr)
						break;

					//auto v = std::lower_bound( tmp_node->nodes, tmp_node->nodes + tmp_node->size, word[offset] );
					index = binary_search( tmp_node, word[offset] );
					if(index == -1)
						break;
					else
						tmp_node = obj_pool->get( tmp_node->nodes[index].second );
						//tmp_node = obj_pool->get( tmp_node->get( index ) );
				}

				*node = tmp_node;

				return offset;
			}

			/***/
			inline void print_all()
			{
				root.print_all( 0, obj_pool, f_manager );
			}

			// TODO creare la versione ricorsiva in cui torna indietro fino al nodo colpevole
			/** adds a node to the trie
			 *
			 * @param word   - word to add
			 * @param length - length of the word
			 * @param index  - position in the scores array
			*/
			inline void addNode( const string& word, const uint32_t length, const uint64_t index )
			{
				node_t* node = &root, *tmp_node;
				uint64_t offset, mismatch;

				offset = scan_trie( &node, word, length );

				// avoid the scan of the trie, since there is only the root
				if(offset == 0){
					insert_node( node, -1, word[0], word, length, index );
					return;
				}

				string s = get_string_on_leaf( node );

				// if the two strings are the same it will not be added
				if(get_index_mismatch( word, length, s, s.length(), &mismatch ) == -1)
					return;

				node = &root;
				int64_t _index = -1;
				bool resized = 0;
				for(offset = 0; ; offset = offset + node->offset){
					if(offset == mismatch){
						// if the value is equal to the offset of a node, just add the node and exit
						if(offset == length)
							node->file_offset = f_manager.write( word, word.length() );
						else
							insert_node( node, _index, word[offset], word, length - offset, index );

						break;
					}
					else{
						if(offset > mismatch){
							uint32_t diff = offset - mismatch;
							node->offset = node->offset - diff;

							// creates a new node copying the children nodes of the mismatching one
							uint64_t idx;
							tmp_node = obj_pool->allocate( 1, &resized, &idx );
							if(resized){
								node = obj_pool->get( _index );
								resized = 0;
							}

							if(node->nodes != nullptr){
								// assign the values from the father to the child node
								tmp_node->nodes = node->nodes;
								tmp_node->size = node->size;
							}
							node->nodes = new pair<char, uint64_t>[node->size = 1];

							// add the node to the current one
							node->nodes[0] = { s[mismatch], idx };

							tmp_node->offset = diff;
							tmp_node->file_offset = node->file_offset;
							node->file_offset = -1;
							tmp_node->index = node->index;

							// add the new word
							insert_node( node, _index, word[mismatch], word, length - mismatch, index );

							break;
						}
						else
							node = obj_pool->get( _index = node->nodes[binary_search( node, word[offset] )].second );
							//node = obj_pool->get( _index = node->get( binary_search( node, word[offset] ) ) );
					}
				}
			}

			/***/
			inline void strongPrefix( const string& P, const uint32_t length, vector<string>& list )
			{
				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = scan_trie( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );

				node = &root;
				for(offset = 0; offset < length; offset += node->offset){
					if(offset >= mismatch){
						//cout << "PREFIX SHARED: " + P.substr( 0, offset ) << endl;
						node->retrieveData( list, obj_pool, f_manager );
						return;
					}
					else
						node = obj_pool->get( node->nodes[binary_search( node, P[offset] )].second );
				}

				node->retrieveData( list, obj_pool, f_manager );
			}

			/***/
			inline void weakPrefix( const string& P, const uint32_t length, vector<string>& list )
			{
				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = scan_trie( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );
				if(mismatch < length){
					cout << "prefix " + P + " not found" << endl;
					return;
				}

				node = &root;
				for(offset = 0; offset < length; offset += node->offset)
					node = obj_pool->get( node->nodes[binary_search( node, P[offset] )].second );

				node->retrieveData( list, obj_pool, f_manager );
			}

			/***/
			void finish()
			{
				obj_pool->finish();
				f_manager.finish();
			}

			/***/
			void deleteNode( const string& P, const unsigned int length ); //TODO

			void setDFOUDS(); //TODO
	};

	/* implemented with minimum perfect hashing */
	class patriciaTrie3
	{
		private:
			#pragma pack( 1 )
			class node_t
			{
				public:
					/* list of attached nodes */
					min_perfect_hashing<uint64_t>* nodes = nullptr;
					/* position of the word on disk */
					int64_t file_offset;
					/* offset in the string respect to the initial position */
					uint16_t offset;
					/* position in the scores array */
					int64_t index;

					node_t() : file_offset( -1 ), offset( 0 ), index( -1 )
					{

					}

					~node_t()
					{
						if(nodes != nullptr){
							// FIXME sistemare
							/*uint8_t size = nodes->getSize();
							for(uint8_t i = 0; i < size; i++)
								nodes->at( i )->~node_t();

							delete nodes;*/
						}
					}

					/***/
					inline void init()
					{
						nodes = nullptr;
						file_offset = -1;
						offset = 0;
						index = -1;
					}

					/*void print_all( unsigned int level )
					{
						node* tmp_node;
						for(pair<const char, node_t*>& item : *nodes){
							tmp_node = item.second;
							cout << "FROM: " << level << ", SIZE: " << nodes->size() << ", ";
							//if(tmp_node->word != nullptr)
								//cout << "WORD: " << *(tmp_node->word) << ", ";
							cout << "LEVEL: " << (level + 1) << ", CHAR: " << item.first << ", OFFSET: " << tmp_node->offset << endl;

							tmp_node->print_all( level + 1 );
						}
					}*/

					/***/
					inline void retrieveData( vector<string>& list, objectPool<node_t>* pool, file_manager& file_man )
					{
						if(file_offset >= 0)
							list.push_back( file_man.read( file_offset ) );

						if(nodes != nullptr){
							uint16_t size = nodes->getSize();
							for(uint16_t i = 0; i < size; i++)
								//nodes->at( i )->retrieveData( list, file_man );
								pool->get( *(nodes->at( i )) )->retrieveData( list, pool, file_man );
						}
					}
			};

			/* root of the trie */
			node_t root;
			/* size of the pool */
			static constexpr const uint64_t POOL_SIZE = 1 << 19;
			/* pool of nodes */
			objectPool<node_t>* obj_pool = nullptr;
			/* manager for writes and reads from disk */
			file_manager f_manager;

		public:
			patriciaTrie3()
			{
				obj_pool = new objectPool<node_t>( POOL_SIZE );
			}

			~patriciaTrie3()
			{
				// TODO riaggiungere solo quando tutto è sistemato
				//root.~node_t();
				//obj_pool->~objectPool();
			}

			/***/
			inline int32_t get_index_mismatch( const string& a, const uint64_t a_size,
											   const string& b, const uint64_t b_size,
											   uint64_t* mismatch )
			{
				uint64_t i, l = (a_size < b_size) ? a_size : b_size;
				for(i = 0; i < l; i++){
					if(a[i] != b[i])
						break;
				}

				*mismatch = i;

				// checks if the two strings are the same
				if(a_size == b_size && i == l)
					return -1;
				else
					return 0;
			}

			/***/
			inline string get_string_on_leaf( node_t* node )
			{
				if(node->file_offset == -1){
					// goes down until we found a leaf
					node = obj_pool->get( *(node->nodes->at( 0 )) );
					//cout << "NODE: " << node << endl;
					for(; node->file_offset == -1; node = obj_pool->get( *(node->nodes->at( 0 )) ));
				}

				return f_manager.read( node->file_offset );
			}

			/** add a child to the current node */
			inline void insert_node( node_t* node, const int64_t _index, const char c,
									 const string& word, const uint32_t offset, const uint64_t index )
			{
				uint64_t idx = 0;
				bool resized = 0;
				node_t* _node = obj_pool->allocate( 1, &resized, &idx );

				if(resized && _index >= 0)
					node = obj_pool->get( _index );

				if(node->nodes == nullptr)
					node->nodes = new min_perfect_hashing<uint64_t>( c, new uint64_t( idx ) );
				else
					node->nodes->insert( c, new uint64_t( idx ) );
				//cout << "INSERITO: " << node->nodes->getSize() << endl;

				_node->offset = offset;
				_node->index = index;
				_node->file_offset = f_manager.write( word, word.length() );
			}

			/** scans the trie until it doesn't found a branch */
			inline uint64_t scan_trie( node_t** node, const string& word, const uint32_t length )
			{
				uint64_t offset;
				node_t* tmp_node = *node;
				uint64_t* index;

				for(offset = 0; offset < length; offset += tmp_node->offset){
					//cout << "CHECKING..." << endl;
					if(tmp_node->nodes == nullptr)
						break;

					//cout << "GETTING..." << endl;
					index = tmp_node->nodes->get( word[offset] );
					if(index == nullptr)
						break;

					tmp_node = obj_pool->get( *index );
					//cout << "NODE: " << (n_node == nullptr) << endl;
					//if(n_node == nullptr)
						//break;
					//else
						//tmp_node = n_node;
					//cout << "OFFSET: " << tmp_node->offset << endl;
				}

				//cout << "TMP_NODE: " << tmp_node << endl;
				*node = tmp_node;

				return offset;
			}

			/***/
			void print_all()
			{
				//root.print_all( 0 );
			}

			// TODO creare la versione ricorsiva in cui torna indietro fino al nodo colpevole
			/** adds a node to the trie
			 *
			 * @param word   - word to add
			 * @param length - length of the word
			 * @param index  - position in the scores array
			*/
			inline void addNode( const string& word, const uint32_t length, const uint64_t index )
			{
				node_t* node = &root, *tmp_node;
				uint64_t offset, mismatch;

				//cout << "\nADDING: " << word << endl;

				//cout << "SCANNING..." << endl;
				offset = scan_trie( &node, word, length );
				//cout << "SCANNED" << endl;

				// avoid the scan of the trie, since there is only the root
				if(offset == 0){
					insert_node( node, -1, word[0], word, length, index );
					return;
				}

				//cout << "OTTENGO STRINGA..." << endl;
				string s = get_string_on_leaf( node );
				//cout << "STRINGA: " << s << endl;

				// if the two strings are the same it doesn't add it
				if(get_index_mismatch( word, length, s, s.length(), &mismatch ) == -1)
					return;

				//cout << "MISMATCH: " << mismatch << endl;

				node = &root;
				int64_t _index = -1;
				bool resized = 0;
				for(offset = 0; ; offset = offset + node->offset){
					if(offset == mismatch){
						//cout << "QUI DENTRO UGUALE: " << offset << ", LENGTH: " << length << endl;
						// if the value is equal to the offset of a node, just add the node and exit
						if(offset == length)
							node->file_offset = f_manager.write( word, word.length() );
						else
							insert_node( node, _index, word[offset], word, length - offset, index );

						break;
					}
					else{
						if(offset > mismatch){
							//cout << "QUI DENTRO DIVERSO" << endl;
							uint64_t diff = offset - mismatch;
							node->offset = node->offset - diff;

							// creates a new node copying the children nodes of the mismatching one
							uint64_t idx = 0;
							tmp_node = obj_pool->allocate( 1, &resized, &idx );
							if(resized){
								node = obj_pool->get( _index );
								resized = 0;
							}

							//cout << "SCAMBIO I PUNTATORI..." << endl;
							if(node->nodes != nullptr)
								// assign the values from the father to the child node
								tmp_node->nodes = node->nodes;

							//cout << "INSERISCO" << endl;
							// add the node to the current one
							node->nodes = new min_perfect_hashing<uint64_t>( s[mismatch], new uint64_t( idx ) );
							//cout << "INSERITO" << endl;

							tmp_node->offset = diff;
							tmp_node->file_offset = node->file_offset;
							node->file_offset = -1;
							tmp_node->index = node->index;

							// add the new word
							insert_node( node, _index, word[mismatch], word, length - mismatch, index );

							break;
						}
						else
							node = obj_pool->get( _index = *(node->nodes->get( word[offset] )) );
					}
				}
			}

			/***/
			inline void strongPrefix( const string& P, const uint32_t length, vector<string>& list )
			{
				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = scan_trie( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );

				node = &root;
				for(offset = 0; offset < length; offset += node->offset){
					if(offset >= mismatch){
						//cout << "PREFIX SHARED: " + P.substr( 0, offset ) << endl;
						node->retrieveData( list, obj_pool, f_manager );
						return;
					}
					else
						node = obj_pool->get( *(node->nodes->get( P[offset] )) );
				}

				node->retrieveData( list, obj_pool, f_manager );
			}

			/***/
			inline void weakPrefix( const string& P, const uint32_t length, vector<string>& list )
			{
				node_t* node = &root;
				uint64_t offset, mismatch;

				offset = scan_trie( &node, P, length );
				if(offset == 0)
					return;

				string s = get_string_on_leaf( node );

				get_index_mismatch( P, length, s, s.length(), &mismatch );
				if(mismatch < length){
					cout << "prefix " + P + " not found" << endl;
					return;
				}

				node = &root;
				for(offset = 0; offset < length; offset += node->offset)
					node = obj_pool->get( *(node->nodes->get( P[offset] )) );

				node->retrieveData( list, obj_pool, f_manager );
			}

			/***/
			void finish()
			{
				//TODO obj_pool->finish();
				f_manager.finish();
			}

			/***/
			void deleteNode( const string& P, const unsigned int length ); //TODO

			void setDFOUDS(); //TODO
	};

	class ternary_search_tree
	{
		private:
			enum Position{ LESS, EQUALS, GREATER };

			class node_t
			{
				// TODO trasformare in private
				public:
					/**/
					char c = -1;
					/**/
					int64_t file_offset = -1;
					/**/
					int64_t* edges = nullptr;

				public:
					node_t(){}
					~node_t(){}

					/***/
					inline void init()
					{
						c = -1;
						file_offset = -1;
						edges = nullptr;
					}

					/***/
					inline int64_t next( const char value )
					{
						if(edges == nullptr)
							return -1;

						// TODO istruzione vettoriale
						if(value < c) return edges[LESS];
						if(value == c) return edges[EQUALS];
						return edges[GREATER];
					}

					/***/
					inline void setValue( const char value )
					{
						c = value;
					}

					/***/
					inline void set_string( const uint64_t offset )
					{
						file_offset = offset;
					}

					/***/
					inline void addNode( const Position pos, const int64_t index )
					{
						if(edges == nullptr){
							edges = new int64_t[3];
							edges[0] = edges[1] = edges[2] = -1;
						}

						edges[pos] = index;
					}

					/***/
					inline void retrieve_data( vector<string>& list, objectPool<node_t>* pool , file_manager* file_mgr )
					{
						if(file_offset >= 0)
							list.push_back( file_mgr->read( file_offset ) );

						if(edges == nullptr)
							return;

						// ricorsion
						int64_t index;
						if((index = edges[LESS]) >= 0) pool->get( index )->retrieve_data( list, pool, file_mgr );
						if((index = edges[EQUALS]) >= 0) pool->get( index )->retrieve_data( list, pool, file_mgr );
						if((index = edges[GREATER]) >= 0) pool->get( index )->retrieve_data( list, pool, file_mgr );
					}
			};

			/* size of the pool */
			static constexpr const uint64_t POOL_SIZE = 1 << 19;
			/* pool of nodes */
			objectPool<node_t>* obj_pool = nullptr;
			/**/
			file_manager f_mgr;
			/**/
			node_t* root = nullptr;

			/** random selection */
			inline uint64_t choose_pivot( const uint64_t from, const uint64_t to )
			{
				//return from;
				// TODO
				return from + (rand() % (to - from));
			}

			/***/
			template<typename T>
			inline void swap( T* a, T* b )
			{
				T tmp = *a;
				*a = *b;
				*b = tmp;
			}

			/***/
			inline uint64_t multikey_qs( vector<string> input, const int64_t from, int64_t to, const uint64_t index = 0 )
			{
				uint64_t idx = 0;
				bool resized;
				node_t* _node = obj_pool->allocate( 1, &resized, &idx );
				string s_pivot;

				// the case to - from == 0 is not taken into account (and avoided by the algorithm)
				if(to - from == 1){
					//printf( "SCRIVO NEL FILE: %s:%d\n", (s_pivot = input[from]).c_str(), s_pivot.length() );
					s_pivot = input[from];
					_node->set_string( f_mgr.write( s_pivot, s_pivot.length() ) );
					_node->setValue( s_pivot[index] );

					printf( "INSERITO 1: %s\n", s_pivot.c_str() );
				}
				else{
					int64_t pivot;
					while(true){
						// all the sub-strings are added on the node and then (logically) removed from the vector
						pivot = choose_pivot( from, to );
						//cout << "PIVOT: " << pivot << ", FROM: " << from << ", TO: " << to << endl;
						s_pivot = input[pivot];
						if(index < s_pivot.length())
							break;
						else{
							if(_node->file_offset == -1)
								_node->set_string( f_mgr.write( s_pivot, s_pivot.length() ) );
							// put the string over the last item to logically remove it
							swap<string>( &s_pivot, &(input[--to]) );
						}
					}

					if(to <= from)
						return idx;

					char c_pivot = s_pivot[index];
					if(pivot > from)
						swap<string>( &(input[from]), &(input[pivot]) );

					_node->setValue( c_pivot );

					//printf( "INSERITO 2 [INDEX = %lu]: %c\n", index, c_pivot );

					//for(int64_t i = from; i < to; i++){
						//printf( "[%lu] = %s\n", i, input[i].c_str() );
					//}

					//cout << endl;

					// scan the partition and divide it in 3 sets: <, =, >
					uint64_t size_less = 0, size_equals = 1;
					string curr;
					for(int64_t i = from + 1; i < to; i++){
						if(index > (curr = input[i]).length()){
							if(_node->file_offset == -1){
								//cout << "SCRIVO NEL FILE: " << curr << ":" << curr.length();
								_node->set_string( f_mgr.write( curr, curr.length() ) );
								printf( "INSERITO 2: %s\n", curr.c_str() );
							}
							// put the string over the last item
							swap<string>( &(input[i--]), &(input[--to]) );
						}
						else{
							if(curr[index] < c_pivot){
								// swap 1: put it on top of <
								swap<string>( &(input[i]), &(input[from + size_less++]) );
								// swap 2: put it on top of =
								swap<string>( &(input[i]), &(input[from + size_equals++]) );
							}
							else{
								if(curr[index] == c_pivot){ // put it on top of =
									swap<string>( &(input[i]), &input[from + size_equals++] );
								}
							}
						}
					}

					//for(int64_t i = from; i < to; i++){
						//printf( "[%lu] = %s\n", i, input[i].c_str() );
					//}

					//printf( "LESS: %lu, EQUALS: %lu, GREATER: %lu\n", size_less, size_equals - size_less - 1, to - from - size_equals );

					uint64_t partition = from + size_less;
					//printf( "LESS = %lu - %lu\n", from, partition );
					if(partition - from > 0)
						_node->addNode( LESS, multikey_qs( input, from, partition, index ) );
					//printf( "EQUALS = %lu - %lu\n", partition, partition + (size_equals - size_less) );
					if((partition + (size_equals - size_less)) - partition > 0)
						_node->addNode( EQUALS, multikey_qs( input, partition, partition + (size_equals - size_less), index + 1 ) );
					//printf( "GREATER = %lu - %lu\n", partition + (size_equals - size_less), to );
					if(to - (partition + (size_equals - size_less)) > 0)
						_node->addNode( GREATER, multikey_qs( input, partition + (size_equals - size_less), to, index ) );
				}

				return idx;
			}

		public:
			ternary_search_tree()
			{
				obj_pool = new objectPool<node_t>( POOL_SIZE );
			}

			/** Builds the tree from a vector of strings.
			 *  It makes use of the multikey-quicksort algorithm to have the O(P + log n) time to retrieve a string
			 *
			 * @param input	- input vector
			 * @param size	- input dimension
			*/
			explicit ternary_search_tree( const vector<string> input, const uint64_t size )
			{
				obj_pool = new objectPool<node_t>( POOL_SIZE );

				srand( time( NULL ) );

				multikey_qs( input, 0, size );

				// gets the first allocated node
				root = obj_pool->get( 0 );
			}

			~ternary_search_tree()
			{

			}

			/***/
			inline void addNode( const string value )
			{
				// TODO scandisce l'albero fino a che non termina la stringa o non si raggiunge una foglia

			}

			/***/
			inline void strongPrefix( const string P, const uint64_t size, vector<string>& list )
			{
				node_t* node = root;

				int64_t index;
				uint64_t i = 0;
				while(true){
					printf( "VALUE: %c\n", node->c );
					index = node->next( P[i] );
					if(node->c == P[i] && ++i == size)
						break;

					if(index == -1)
						break;
					else
						node = obj_pool->get( index );
				}

				printf( "SHARED PREFIX: %s\n", P.substr( 0, i ).c_str() );

				if(node->file_offset >= 0){
					printf( "FILE_OFFSET: %li\n", node->file_offset );
					list.push_back( f_mgr.read( node->file_offset ) );
				}

				if(node->edges != nullptr && (index = node->edges[EQUALS]) >= 0)
					obj_pool->get( index )->retrieve_data( list, obj_pool, &f_mgr );
			}

			/***/
			inline void finish()
			{
				obj_pool->finish();
				root = obj_pool->get( 0 );
				f_mgr.finish();
			}
	};
}

#endif /* SEARCHTREE_H_ */
