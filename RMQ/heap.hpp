/*
 * heap.hpp
 *
 *  Created on: 03 nov 2015
 *      Author: stefano
 */

#ifndef HEAP_HPP_
#define HEAP_HPP_

class heap
{
	private:
		/** element of the heap array */
		typedef struct heap_elem{
			/* value of the element */
			uint64_t value;
			/* position in the array */
			uint64_t index;
			/* starting position */
			uint64_t from;
			/* ending position */
			uint64_t to;
		}heap_elem_t;

		/* the array used internally by the heap */
		heap_elem *heap_t;
		/* size of the heap */
		uint64_t heap_size;

		/** returns the index of the father in the queue, starting from the i-th node
		 *
		 * @param i - node index
		 *
		 * @return father index
		*/
		inline uint64_t father( uint64_t i )
		{
			return ((i - 1) >> 1);
		}

		/** returns the index of the left node in the queue, starting from the father
		 *
		 * @param i - node index
		 *
		 * @return left node index
		*/
		inline uint64_t left_node( uint64_t i )
		{
			return ((i << 1) + 1);
		}

		/** swap two elements in the heap
		 *
		 * @param x - first element
		 * @param y - second element
		*/
		inline void swap( heap_elem *x, heap_elem *y )
		{
			heap_elem temp_elem = *x;
			*x = *y;
			*y = temp_elem;
		}

		/** returns the index of the maximum node between father and sons
		 *
		 * @param father - father index
		 *
		 * @return index of the maximum node
		*/
		inline uint64_t max_father_children( uint64_t father )
		{
			uint64_t sx = father, dx;

			sx = dx = left_node( father );
			if(dx + 1 < heap_size) dx++;
			if(heap_t[dx].value > heap_t[sx].value) sx = dx;
			if(heap_t[father].value > heap_t[sx].value) sx = father;

			return sx;
		}

		/** reorganize the heap
		 *
		 * @param i - index from which the reorganization start
		*/
		inline void reorganize_heap( uint64_t i )
		{
			uint64_t select;

			// checks if some node must be put to the top
			while(i > 0 && (heap_t[i].value > heap_t[father( i )].value)){
				swap( &heap_t[i], &heap_t[father( i )] );
				i = father( i );
			}

			// checks if some node must be put on the bottom
			while(left_node( i ) < heap_size && i != max_father_children( i )){
				select = max_father_children( i );
				swap( &heap_t[i], &heap_t[select] );
				i = select;
			}
		}

	public:
		heap( uint64_t K )
		{
			// creates the array
			heap_t = new heap_elem[K << 1];
			heap_size = 0;
		}

		~heap()
		{
			delete[] heap_t;
		}

		/** checks if the queue is empty
		 *
		 * @return 0 if the queue is empty, n (different from 0) otherwise
		*/
		inline int is_empty()
		{
			return !heap_size;
		}

		/** adds an element on the heap
		 *
		 * @param value - value of the item
		 * @param index - index in the array
		 * @param from  - left boundary
		 * @param to    - right boundary
		*/
		inline void enqueue( uint64_t value, uint64_t index, uint64_t from, uint64_t to )
		{
			// puts the element on tail and reorganize the heap

			heap_t[heap_size].value = value;
			heap_t[heap_size].index = index;
			heap_t[heap_size].from = from;
			heap_t[heap_size].to = to;

			reorganize_heap( heap_size++ );
		}

		/** removes the element with the maximum value from the heap
		 *
		 * @return the element
		*/
		inline heap_elem dequeue()
		{
			heap_elem elem;

			// get the first element and reorganize the heap
			elem = heap_t[0];
			swap( &heap_t[0], &heap_t[heap_size-1] );
			heap_size--;

			reorganize_heap( 0 );

			return elem;
		}
};

#endif /* HEAP_HPP_ */
