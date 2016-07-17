/*
 * min_perfect_hashing.hpp
 *
 *  Created on: 04 nov 2015
 *      Author: stefano
 */

#ifndef _MIN_PERFECT_HASHING_HPP_
#define _MIN_PERFECT_HASHING_HPP_

#include "murmur_hash.hpp"

#pragma pack(1)
template<typename T>
class min_perfect_hashing
{
	private:
		// node of the hypergraph
		typedef struct node{
			/**/
			uint16_t degree = 0;
			/**/
			int16_t index = -1;
		} node_t;

		typedef struct value{
			/**/
			uint8_t key = 0;
			/**/
			T* value = nullptr;
			/**/
			int16_t sum = 0;
		} value_t;

		/**/
		static constexpr float C = 1.25;
		/**/
		murmur_hash_type seed1 = 0, seed2 = 1, seed3 = 2;
		/**/
		value_t* hash_table;
		/**/
		uint16_t n;

		/** sets values to seeds */
		inline void set_hash_values()
		{
			seed1++;
			seed2++;
			seed3++;
			/*murmur_hash_type value;
			while(true){
				value = std::rand() % 100;
				if(value != seed1) seed1 = value;
				value = std::rand() % 100;
				if(value != seed2) seed2 = value;
				value = std::rand() % 100;
				if(value != seed3) seed3 = value;

				if(seed1 != seed2 && seed1 != seed3 && seed2 != seed3)
					break;
			}*/
		}

		/** computes the degree of each node generating an hypergraph
		 *
		 * @param nodes - nodes of the hypergraph
		 * @param m		- number of nodes
		*/
		inline void compute_degree( node_t* nodes, const uint16_t& m )
		{
			uint8_t key;
			murmur_hash_type h1, h2, h3;

			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr){
					key = hash_table[i].key;
					h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % m;
					nodes[h1].degree++;
					h2 = murmur_hash::murmurHash3( &key, 1, seed2 ) % m;
					nodes[h2].degree++;
					h3 = murmur_hash::murmurHash3( &key, 1, seed3 ) % m;
					nodes[h3].degree++;
				}
			}

			//cout << "DEGREE CALCOLATI" << endl;
			//for(uint16_t i = 0; i < m; i++)
				//cout << "I: " << i << ", DEGREE: " << (int) nodes[i].degree << endl;
		}

		/** peels the graph choosing and "removing" the 1-degree node
		 *
		 * @param nodes - nodes of the hypergraph
		 * @param m		- number of nodes
		 *
		 * @return 1 if the peeling is done succesfully, 0 otherwise
		*/
		inline bool peel_graph( node_t* nodes, const uint16_t& m )
		{
			uint8_t key;
			murmur_hash_type h1, h2, h3;

			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr){
					key = hash_table[i].key;
					h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % m;
					h2 = murmur_hash::murmurHash3( &key, 1, seed2 ) % m;
					h3 = murmur_hash::murmurHash3( &key, 1, seed3 ) % m;
					if(nodes[h1].degree == 1 && nodes[h1].index == -1){
						nodes[h1].index = i;
						nodes[h2].degree--;
						nodes[h3].degree--;
					}
					else{
						if(nodes[h2].degree == 1 && nodes[h2].index == -1){
							nodes[h2].index = i;
							nodes[h1].degree--;
							nodes[h3].degree--;
						}
						else{
							if(nodes[h3].degree == 1 && nodes[h3].index == -1){
								nodes[h3].index = i;
								nodes[h1].degree--;
								nodes[h2].degree--;
							}
							else{
								// if all the possibilites are taken it will be rebuilded using different hashes
								//cout << "REBUILD ALL" << endl;
								set_hash_values();

								// reset the values
								for(i = 0; i < m; i++){
									nodes[i].degree = 0;
									nodes[i].index = -1;
								}

								return 0;
							}
						}
					}
				}
			}

			//cout << "GRAPH PEELED" << endl;
			//for(uint16_t i = 0; i < m; i++)
				//cout << "I: " << i << ", INDEX: " << (int) nodes[i].index << endl;

			return 1;
		}

		/** builds the hash table in reverse order respect to the peeling
		 *
		 * @param nodes - nodes of the hypergraph
		 * @param m		- number of nodes
		*/
		inline void build_hash( const node_t* nodes, const uint16_t& m )
		{
			uint16_t free_c[3];
			uint8_t size = 0;
			int16_t sum, index;
			murmur_hash_type h1, h2, h3;
			char key;

			bool* free_cell = new bool[m];
			memset( free_cell, 1, m * sizeof( bool ) );

			value_t* table = new value_t[m];

			for(int16_t i = n - 1; i >= 0; i--){
				if(hash_table[i].value != nullptr){
					key = hash_table[i].key;
					h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % m;
					h2 = murmur_hash::murmurHash3( &key, 1, seed2 ) % m;
					h3 = murmur_hash::murmurHash3( &key, 1, seed3 ) % m;

					//cout << "H1: " << h1 << ", H2: " << h2 << ", H3: " << h3 << endl;

					if(nodes[h1].index == i) index = h1;
					else if(nodes[h2].index == i) index = h2;
					else index = h3;

					//cout << "KEY: " << value << ", INDEX: " << (int) index << endl;

					sum = size = 0;
					if(free_cell[h1]){ free_c[size++] = h1; free_cell[h1] = 0; }
					else sum += table[h1].sum;
					if(free_cell[h2]){ free_c[size++] = h2; free_cell[h2] = 0; }
					else if(h2 != h1) sum += table[h2].sum;
					if(free_cell[h3]){ free_c[size++] = h3; free_cell[h3] = 0; }
					else if(h3 != h2 && h3 != h1) sum += table[h3].sum;

					//cout << "SUM: " << sum << endl;

					//for(uint16_t j = 0; j < free_c.size(); j++)
						//cout << "J: " << j << ", FREE: " << free_c[j] << endl;

					for(uint16_t j = 0; j < size; j++){
						table[free_c[j]].sum = (index - sum);
						sum = index;
					}

					//for(uint16_t j = 0; j < m; j++)
						//cout << "J: " << j << ", TABLE: " << table[j].sum << endl;

					if(sum != index)
						cerr << "ERROR: " << sum << " - " << index << endl;
					else{
						//cout << "INSERITO: " << key << ", IN POSIZIONE: " << index << endl;
						table[index].value = hash_table[i].value;
						table[index].key = key;
					}
				}
			}

			delete[] free_cell;
			delete[] hash_table;
			hash_table = table;
		}

	public:
		inline explicit min_perfect_hashing()
		{
			hash_table = new value_t[n = (int16_t) C];
		}

		/** creates the hash adding the input value
		 *
		 * @param c		- where the item is inserted
		 * @param value	- value associated
		*/
		inline explicit min_perfect_hashing( uint8_t c, T* value )
		{
			set_hash_values();

			hash_table = new value_t[n = (int16_t) C];

			insert( c, value );
		}

		~min_perfect_hashing()
		{
			clear();
		}

		/**  */
		inline void insert( const uint8_t key, T* val )
		{
			// if it is already present it will not be added
			if(lookup( key ))
				return;

			if(n == 1){
				// adds in a faster way
				murmur_hash_type h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % n;

				hash_table[h1].key = key;
				hash_table[h1].value = val;
				hash_table[h1].sum = h1;

				return;
			}

			//cout << "N: " << n << endl;

			// insert the key in the first free cell
			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value == nullptr){
					hash_table[i].key = key;
					hash_table[i].value = val;
					break;
				}
			}

			uint16_t m = (uint16_t) (C * (n + 1));

			// creates the list of graph nodes
			node_t* nodes = new node_t[m];

			while(true){
				//cout << "CALCOLO I DEGREE.." << endl;
				compute_degree( nodes, m );
				//cout << "DEGREE CALCOLATI" << endl;

				//cout << "PEELING GRAPH..." << endl;
				if(peel_graph( nodes, m ))
					break;
			}
			//cout << "GRAPH PEELED" << endl;

			build_hash( nodes, m );

			// update the size
			n = m;

			delete[] nodes;

			//for(uint64_t i = 0; i < n; i++)
				//cout << "HASH[" << i << "]: " << hash_table[i].sum << endl;
		}

		/***/
		inline bool lookup( const uint8_t& key )
		{
			murmur_hash_type h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % n;
			murmur_hash_type h2 = murmur_hash::murmurHash3( &key, 1, seed2 ) % n;
			murmur_hash_type h3 = murmur_hash::murmurHash3( &key, 1, seed3 ) % n;

			//cout << "H1: " << h1 << ", H2: " << h2 << ", H3: " << h3 << endl;

			int16_t index = hash_table[h1].sum;
			if(h1 == h2){
				if(h1 != h3)
					index += hash_table[h3].sum;
			}
			else{
				index += hash_table[h2].sum;
				if(h1 != h3 && h2 != h3)
					index += hash_table[h3].sum;
			}

			//cout << "INDEX: " << index << ", VALUE: " << hash_table[index].key << endl;

			if(index >= 0 && index < n && hash_table[index].key == key)
				return true;
			else
				return false;
		}

		/***/
		inline T* get( const uint8_t& key )
		{
			murmur_hash_type h1 = murmur_hash::murmurHash3( &key, 1, seed1 ) % n;
			murmur_hash_type h2 = murmur_hash::murmurHash3( &key, 1, seed2 ) % n;
			murmur_hash_type h3 = murmur_hash::murmurHash3( &key, 1, seed3 ) % n;

			int16_t index = hash_table[h1].sum;
			if(h1 == h2){
				if(h1 != h3)
					index += hash_table[h3].sum;
			}
			else{
				index += hash_table[h2].sum;
				if(h1 != h3 && h2 != h3)
					index += hash_table[h3].sum;
			}

			//cout << "CERCO: " << key << ", IN POSIZIONE: " << index << endl;
			if(index >= 0 && index < n && hash_table[index].key == key)
				return hash_table[index].value;
			else
				return nullptr;
		}

		/***/
		T* at( const uint16_t& index )
		{
			//cout << "SIZE: " << size << endl;
			uint16_t j = 0;
			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr){
					if(j == index)
						return hash_table[i].value;
					else
						j++;
				}
			}

			return nullptr;
		}

		/***/
		T& operator []( const uint16_t& index )
		{
			uint16_t j = 0;
			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr){
					if(j == index)
						return hash_table[i].value;
					else
						j++;
				}
			}

			return nullptr;
		}

		/***/
		inline std::ostream& operator<<( std::ostream& stream )
		{
			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr){
					// TODO scrivere cosa c'è dentro
					stream << "";
				}
			}

			return stream;
		}

		/***/
		inline void clear()
		{
			delete[] hash_table;
		}

		/***/
		inline uint16_t getSize()
		{
			uint16_t size = 0;
			for(uint16_t i = 0; i < n; i++){
				if(hash_table[i].value != nullptr)
					size++;
			}

			return size;
		}
};

#endif /* _MIN_PERFECT_HASHING_HPP_ */
