
/*
 * murmur_hash.hpp
 *
 *  Created on: 04 nov 2015
 *      Author: stefano
 */

#ifndef _MURMUR_HASH_HPP_
#define _MURMUR_HASH_HPP_

/* type used by the murmur hash function */
typedef uint32_t murmur_hash_type;

class murmur_hash
{
	public:
		//-----------------------------------------------------------------------------
		// MurmurHash2, by Austin Appleby

		// Note - This code makes a few assumptions about how your machine behaves -

		// 1. We can read a 4-byte value from any address without crashing
		// 2. sizeof(int) == 4

		// And it has a few limitations -

		// 1. It will not work incrementally.
		// 2. It will not produce the same results on little-endian and big-endian
		//    machines.

		inline static murmur_hash_type murmurHash2( const void* key, int len, const murmur_hash_type seed )
		{
			// 'M' and 'R' are mixing constants generated offline.
			// They're not really 'magic', they just happen to work well.

			const murmur_hash_type M = 0x5bd1e995;
			const murmur_hash_type R = 24;

			// Initialize the hash to a 'random' value

			murmur_hash_type h = seed ^ len;

			// Mix 4 bytes at a time into the hash

			const uint8_t* DATA = (const uint8_t*) key;

			while(len >= 4){
				murmur_hash_type k = *(murmur_hash_type*) DATA;

				k *= M;
				k ^= k >> R;
				k *= M;

				h *= M;
				h ^= k;

				DATA += 4;
				len -= 4;
			}

			// Handle the last few bytes of the input array
			switch( len ){
				case 3: h ^= DATA[2] << 16;
						/* no break */
				case 2: h ^= DATA[1] << 8;
						/* no break */
				case 1: h ^= DATA[0];
						h *= M;
			};

			// Do a few final mixes of the hash to ensure the last few
			// bytes are well-incorporated.

			h ^= h >> 13;
			h *= M;
			h ^= h >> 15;

			return h;
		}

		/***/
		inline static murmur_hash_type murmurHash3( const void* key, const murmur_hash_type len, const murmur_hash_type seed )
		{
			static const murmur_hash_type c1 = 0xcc9e2d51;
			static const murmur_hash_type c2 = 0x1b873593;
			static const murmur_hash_type r1 = 15;
			static const murmur_hash_type r2 = 13;
			static const murmur_hash_type  m = 5;
			static const murmur_hash_type  n = 0xe6546b64;

			murmur_hash_type hash = seed;

			const murmur_hash_type nblocks = len / 4;
			const murmur_hash_type *blocks = (const murmur_hash_type *) key;
			murmur_hash_type k;
			for(murmur_hash_type i = 0; i < nblocks; i++){
				k = blocks[i];
				k *= c1;
				k = (k << r1) | (k >> (32 - r1));
				k *= c2;

				hash ^= k;
				hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
			}

			const uint8_t* DATA = (const uint8_t*) key;
			const uint8_t *tail = (const uint8_t *) (DATA + nblocks * 4);
			murmur_hash_type k1 = 0;
		
			switch(len & 3){
				case 3:
					k1 ^= tail[2] << 16;
					/* no break */
				case 2:
					k1 ^= tail[1] << 8;
					/* no break */
				case 1:
					k1 ^= tail[0];
					k1 *= c1;
					k1 = (k1 << r1) | (k1 >> (32 - r1));
					k1 *= c2;
					hash ^= k1;
			}

			hash ^= len;
			hash ^= (hash >> 16);
			hash *= 0x85ebca6b;
			hash ^= (hash >> 13);
			hash *= 0xc2b2ae35;
			hash ^= (hash >> 16);
	
			return hash;
		}
};

#endif /* _MURMUR_HASH_HPP_ */
