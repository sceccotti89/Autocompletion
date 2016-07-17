/*
 * objectPool.h
 *
 *  Created on: 17 ott 2015
 *      Author: stefano
 */

#ifndef OBJECTPOOL_HPP_
#define OBJECTPOOL_HPP_

#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <mutex>

// TODO gestirlo tramite lista di blocchi liberi

/** creates a pool of type T objects */
template<typename T>
class objectPool
{
	private:
		/**/
		uint64_t size = 0;
		/**/
		uint64_t pointer = 0;
		/**/
		T* memory_pool;
		/**/
		std::mutex mux;

	public:
		objectPool( const uint64_t init_size ) : size( init_size )
		{
			memory_pool = (T*) malloc( sizeof( T ) * init_size );
		}

		~objectPool()
		{
			free( memory_pool );
		}

		/***/
		inline T* allocate( const uint64_t n_size, bool *resized, uint64_t* index, const bool exclusive = false )
		{
			// TODO se viene implementata la versione full parallel va rimosso il mutex
			if(exclusive)
				mux.lock();

			if(pointer + n_size >= size){
				*resized = 1;
				//std::cout << "RADDOPPIO: " << size << std::endl;

				T* _memory_pool = (T*) malloc( sizeof( T ) * (size << 1) );
				if(_memory_pool == NULL){
					std::cout << "ERRORE NEL RADDOPPIARE LA STRUTTURA" << std::endl;
					return nullptr;
				}

				memcpy( _memory_pool, memory_pool, sizeof( T ) * size );
				free( memory_pool );
				memory_pool = _memory_pool;

				size = size << 1;
			}

			T* m_block = &(memory_pool[*index = pointer]);
			pointer = pointer + n_size;

			// TODO se viene implementata la versione full parallel va rimosso il mutex
			if(exclusive)
				mux.unlock();

			m_block->init();

			return m_block;
		}

		/***/
		inline void deallocate( T* object )
		{
			auto pos = object - memory_pool;
			pointer = pointer - sizeof( *object );
			free( object );
		}

		/***/
		inline T* get( const uint64_t index )
		{
			//std::cout << "INDEX: " << index << ", SIZE: " << pointer << std::endl;
			return &(memory_pool[index]);
		}

		/***/
		inline void merge( objectPool<T>* other )
		{
			T* _memory_pool = (T*) malloc( sizeof( T ) * (pointer + other->pointer) );
			memcpy( _memory_pool, memory_pool, sizeof( T ) * pointer );
			memcpy( _memory_pool + pointer, other->memory_pool, sizeof( T ) * other->pointer );

			free( memory_pool );
			memory_pool = _memory_pool;
			size = pointer = pointer + other->pointer;
		}

		/***/
		inline void finish()
		{
			if(pointer < size){
				T* _memory_pool = (T*) malloc( sizeof( T ) * (size = pointer) );
				memcpy( _memory_pool, memory_pool, sizeof( T ) * size );
				free( memory_pool );
				memory_pool = _memory_pool;
			}
		}
};

#endif /* OBJECTPOOL_H_ */
