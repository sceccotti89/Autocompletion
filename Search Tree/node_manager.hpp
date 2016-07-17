/*
 * node_manager.hpp
 *
 *  Created on: 26 ott 2015
 *      Author: stefano
 */

#ifndef NODE_MANAGER_HPP_
#define NODE_MANAGER_HPP_

#include <vector>

#include "objectPool.hpp"

class node_manager
{
	private:
		/* list of nodes */
		std::vector<std::pair<uint16_t, uint64_t>>* childs = nullptr;
		/* list of pair (first, last) item of each bin */
		std::vector<std::pair<int16_t, int16_t>>* bins = nullptr;

		/***/
		inline int32_t binary_search( int16_t min, int16_t max, const uint16_t key )
		{
			int16_t mid = 0;

			//cout << "MAX: " << max << ", KEY: " << key << endl;

			while(min <= max){
				mid = (max + min) / 2;

				if(mid == key)
					return mid;
				else if(mid < key)
					min = mid + 1;
				else
					max = mid - 1;
			}

			return -1;
		}

	public:
		node_manager(){}
		~node_manager(){}

		/***/
		inline void add_node( const uint16_t value, const uint64_t position )
		{
			if(childs == nullptr || bins == nullptr){
				childs = new std::vector<std::pair<uint16_t, uint64_t>>();
				bins = new std::vector<std::pair<int16_t, int16_t>>();

				childs->push_back( { value, position } );
				bins->push_back( { 0, 0 } );
			}
			else{
				// TODO la prima fase e' quella di aggiungerlo al vettore (usare la ricerca binaria)

				// TODO la seconda di aggiornare la lista di bin (se necessario :: se cambia cioè la sua dimensione)
			}
		}

		/***/
		inline void interpolation_search( uint16_t value )
		{
			// gets the corresponding bin
			uint16_t j = (value - childs->at( 0 ).first) / bins->size() + 1;

			if(bins->at( j ).first == -1 || bins->at( j ).second == -1)//TODO ritorna -1 perché il numero non c'è
				return;
			else{
				//TODO binary search over the bin
			}
		}
};

#endif /* NODE_MANAGER_HPP_ */
