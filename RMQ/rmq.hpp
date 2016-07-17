/*
 * rmq.hpp
 *
 *  Created on: 30 ott 2015
 *      Author: stefano
 */

#ifndef RMQ_HPP_
#define RMQ_HPP_

#include <vector>
#include <cmath>
#include <cassert>

#include "heap.hpp"

// TODO first solution: O(n log(n)) space and O(log n) time
class rmq
{
	private:
		/* list of values */
		std::vector<uint64_t> values;
		/* number of items */
		uint64_t size;
		/* lookup table */
		uint64_t** M = nullptr;
		/* dimension of a sigle line */
		uint8_t t_size;
		/* dimension of a block */
		uint64_t block;
		/* indirection array */
		uint8_t* R;
		/* R dimension */
		uint8_t dim;
		/* the max heap */
		heap* max_heap;
		/* list of top k queries */
		std::vector<uint64_t> topk;

		/* MACRO defintion of maximum */
		#define m_max( a, b ) ((a) > (b))? (a) : (b)
		/* MACRO defintion of minimum */
		#define m_min( a, b ) ((a) < (b))? (a) : (b)

		/***/
		inline void fill_table()
		{
			uint8_t i, j;

			/*for(i = 0; i < dim; i++)
				for(j = 0; j < t_size; j++)
					M[i][j] = j;

			for(j = 1; j < t_size; j++){
				for (i = 0; i + (1 << j) - 1 < dim; i++){
					if(R[M[i][j-1]] >= R[M[i + (1 << (j - 1)) - 1][j-1]])
						M[i][j] = M[i][j-1];
					else
						M[i][j] = M[i + (1 << (j - 1)) - 1][j-1];
				}
			}*/

			/*for (i = 0; i < dim; i++)
				M[i][0] = i;

			for(j = 1; 1 << j <= dim; j++){
				for(i = 0; i + (1 << j) - 1 < dim; i++){
					if(values[R[M[i][j - 1]]] >= values[R[M[i + (1 << (j - 1))][j - 1]]])
						M[i][j] = M[i][j - 1];
					else
						M[i][j] = M[i + (1 << (j - 1))][j - 1];
				}
			}*/

			//memset( M, 0, sizeof( M ) );

			/*for(i = 0; i < dim; i++)
				M[i][0] = i;

			// i is the column, j is the row, every values for a column can be computed (only) from last column
			for(i = 1; i <= t_size; i++){
				for(j = 0; j + (1 << (i-1)) < dim; j++){
					M[j][i] = m_max( M[j][i-1], M[j + (1 << (i-1))][i-1] );
				}
			}*/

			for(i = 0; i < dim; i++){
				for(j = 0; j < t_size && (i + (1 << j) < dim); j++){
					//cout << "POS: " << i + (1 << j) << ", VALUE1: " << (int) R[i] << ", VALUE2: " << (int) R[i + (1 << j)] << endl;
					if(values[R[i]] >= values[R[i + (1 << j)]])
						M[i][j] = i;
					else
						M[i][j] = i + (1 << j);
				}
			}

			//cout << "MATRIX..." << endl;
			//for(i = 0; i < dim; i++){
				//for(j = 0; j < t_size; j++)
					//cout << M[i][j] << " " << endl;

				//cout << endl;
			//}
		}

		/***/
		inline void pre_process()
		{
			// creates the indirected array
			double r_size = std::log2( static_cast<double>( size ) );
			if(r_size - (uint8_t) r_size > 0)
				r_size++;

			R = new uint8_t[dim = r_size];
			cout << "DIM: " << (int) dim << endl;
			block = size / dim;
			cout << "BLOCK: " << block << endl;

			// fills R with the index of the maximum value in each block
			uint8_t j = 0, max = 0;
			for(uint64_t i = 1; i < size; i++){
				if(i % block == 0){
					R[j++] = max;
					cout << "J: " << (j - 1) << ", MAX: " << (int) R[j-1] << endl;
					max = i;
				}
				else{
					if(values[i] > values[max])
						max = i;
				}
			}

			R[j] = max;
			cout << "J: " << (int) j << ", MAX: " << (int) R[j] << endl;

			// compute the size of a table line
			t_size = std::log2( static_cast<double>( dim ) ) + 1;
			cout << "T_SIZE: " << (int) t_size << endl;

			M = new uint64_t*[dim];
			for(uint8_t i = 0; i < dim; i++)
				M[i] = new uint64_t[t_size];

			cout << "START FILLING TABLE" << endl;
			fill_table();
			cout << "TABLE FILLED" << endl;
		}

		/** returns the maximum checking the table
		 *
		 * @param from	- starting position
		 * @param to	- ending position
		 *
		 * @return index of the maximum value
		*/
		inline uint64_t get_table_maximum( uint8_t from, uint8_t to )
		{
			// FIXME ho dovuto aggiungere questo controllo perché per posizioni contigue non funzionava
			if(to == from + 1)
				return (values[R[from]] >= values[R[to]]) ? R[from] : R[to];

			//cout << "FROM: " << (int) from << ", TO: " << (int) to << endl;

			uint8_t len = std::log2( to - from + 1 );
			uint8_t offset = (1 << len);

			//cout << "LEN: " << (int) len << endl;
			//cout << "OFFSET: " << (int) offset << endl;

			//cout << "POSITION: " << M[from][len] << ", FIRST: " << (int) R[M[from][len]] << endl;
			//cout << "POSITION: " << M[to - offset][len] << ", SECOND: " << (int) R[M[to - offset][len]] << endl;

			uint8_t i_from = M[from][len];
			uint8_t i_to = M[to - offset][len];
			if(values[R[i_from]] >= values[R[i_to]])
				return i_from;
			else
				return i_to;

			//return m_max( R[M[from][from + pow]], R[M[to - pow][to]] );
		}

		/** returns the maximum scanning the [from - to] portion
		 *
		 * @param from	- starting position
		 * @param to	- ending position
		 *
		 * @return index of the maximum value
		*/
		inline uint64_t get_scan_maximum( uint64_t from, uint64_t to )
		{
			uint64_t index = from;

			for(uint64_t i = from + 1; i <= to; i++){
				if(values[i] > values[index])
					index = i;
			}

			return index;
		}

		/***/
		/*inline void cartesian_tree( uint64_t from, uint64_t to, uint64_t k )
		{
			uint64_t index, index2;
			index = query( from, to );
			max_heap->enqueue( values[index], index, from, to );

			for(uint64_t i = 0; i < k; i++){
				// extracts the maximum from the heap
				auto elem = max_heap->dequeue();
				cout << "ESTRATTO: " << elem.index << " RANGE: " << elem.from << " - " << elem.to << endl;
				index = elem.index;
				topk.push_back( index );

				// inserts the two maximum values from the extracted one
				if(index - elem.from > 0){
					index2 = query( elem.from, index - 1 );
					max_heap->enqueue( values[index2], index2, elem.from, index - 1 );
					cout << "AGGIUNTO: " << index2 << " IN: " << elem.from << " - " << index - 1 << endl;
				}

				if(elem.to - index > 0){
					index2 = query( index + 1, elem.to );
					max_heap->enqueue( values[index2], index2, index + 1, elem.to );
					cout << "AGGIUNTO: " << index2 << " IN: " << index + 1 << " - " << elem.to << endl;
				}
			}
		}*/

	public:
		rmq( std::vector<uint64_t> values )
		{
			this->values = values;
			this->size = values.size();

			pre_process();
		}

		~rmq(){}

		/***/
		inline uint64_t query( uint64_t from, uint64_t to )
		{
			assert( to < size );

			// compute the first and last block of the query
			uint8_t first_block = from / block;
			uint8_t last_block = to / block;
			uint8_t pos;
			uint64_t max1, max2, max3;

			//cout << "FIRST: " << (int) first_block << endl;
			//cout << "LAST: " << (int) last_block << endl;
			if(first_block == last_block){
				// same block
				if(from % block == 0 && (to == size - 1 || to % block == block - 1)) // all the block
					return R[first_block];
				else // a part of it
					return get_scan_maximum( from, to );
			}

			// first block
			pos = from % block;
			//cout << "POS_FIRST: " << (int) pos << endl;
			if(pos == 0) // if it takes the entire block we use directly the indirection
				max1 = R[first_block];
			else
				max1 = get_scan_maximum( from, m_min( size, from + block - 1 - pos ) );

			//cout << "CALCOLATO MAX1 TRA: " << from << " - " << from + block - 1 - pos << endl;
			//cout << "MAX1: " << max1 << endl;

			//cout << "BLOCCHI INTERMEDI: " << (last_block - first_block - 1) << endl;
			if(last_block - first_block - 1 > 0){
				//cout << "FIRST: " << (int) R[first_block + 1] << ", LAST: " << (int) R[last_block - 1] << endl;
				max2 = get_table_maximum( first_block + 1, last_block - 1 );
			}
			else
				max2 = max1; // max2 has no influence on the final result

			//cout << "MAX2: " << max2 << endl;

			// last block
			pos = to % block;
			//cout << "POS_LAST: " << (int) pos << endl;
			if(pos == 0) // if it takes the entire block we use directly the indirection
				max3 = R[last_block];
			else
				max3 = get_scan_maximum( to - pos, to );

			//cout << "MAX3: " << (int) max3 << endl;
			return (values[max1] >= values[max2]) ?
					((values[max1] >= values[max3]) ? max1 : max3) :
					((values[max2] >= values[max3]) ? max2 : max3);

			/*if(values[max1] >= values[max2]){
				if(values[max1] >= values[max3])
					return max1;
				else
					return max3;
			}
			else{
				if(values[max2] >= values[max3])
					return max2;
				else
					return max3;
			}*/
			//return m_max( m_max( max1, max2 ), max3 );
		}

		/***/
		inline std::vector<uint64_t> top_k( uint64_t from, uint64_t to, uint64_t k )
		{
			assert( to < size && k <= (from - to));

			topk.clear();
			max_heap = new heap( k );

			uint64_t index, index2;
			index = query( from, to );
			max_heap->enqueue( values[index], index, from, to );

			for(uint64_t i = 0; i < k; i++){
				// extracts the maximum from the heap
				auto elem = max_heap->dequeue();
				cout << "ESTRATTO: " << elem.index << " RANGE: " << elem.from << " - " << elem.to << endl;
				index = elem.index;
				topk.push_back( index );

				// inserts the two maximum values from the extracted one
				if(index - elem.from > 0){
					index2 = query( elem.from, index - 1 );
					max_heap->enqueue( values[index2], index2, elem.from, index - 1 );
					cout << "AGGIUNTO: " << index2 << " IN: " << elem.from << " - " << index - 1 << endl;
				}

				if(elem.to - index > 0){
					index2 = query( index + 1, elem.to );
					max_heap->enqueue( values[index2], index2, index + 1, elem.to );
					cout << "AGGIUNTO: " << index2 << " IN: " << index + 1 << " - " << elem.to << endl;
				}
			}

			max_heap->~heap();
			free( max_heap );

			return topk;
		}
};

#endif /* RMQ_HPP_ */
