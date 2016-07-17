/*
 * file_manager.hpp
 *
 *  Created on: 24/ott/2015
 *      Author: stefano
 */

#ifndef FILE_MANAGER_HPP_
#define FILE_MANAGER_HPP_

#include <fstream>
#include <string>
#include <string.h>
#include <mutex>

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

typedef uint32_t p_type;

class file_manager
{
	private:
		/* dimension of the buffer */
		static constexpr const p_type BUFFER_SIZE = 1 << 21;
		/* double-buffering technique */
		char* buffer;
		/* starting and ending position in the buffer */
		int64_t start, end;
		/* path to the external memory file */
		string path;
		/* output file descriptor */
		ofstream ofile;
		/* input file descriptor */
		ifstream ifile;
		/* mutex used to access the buffer or the file */
		mutex mux;

	public:
		file_manager( const string PATH = "strings.txt" )
		{
			start = end = 0;

			path = PATH;
			ofile.open( PATH );
			ifile.open( PATH );

			buffer = (char*) calloc( BUFFER_SIZE, sizeof( char ) );
		}

		~file_manager()
		{
			ofile.close();
			ifile.close();
		}

		/** writes on disk the word */
		inline uint64_t write( const string& word, const uint64_t length, const bool exclusive = false )
		{
			// TODO se viene implementata la versione full parallel va rimosso il mutex
			if(exclusive)
				mux.lock();

			if(length + 1 > BUFFER_SIZE)
				ofile.write( (word + "\n").c_str(), length + 1 );
			else{
				uint64_t size = end - start;
				p_type end_pos = size + length + 1;
				if(end_pos >= BUFFER_SIZE){
					ofile.write( buffer, size );
					ofile.flush();

					memset( buffer, '\0', size );

					start = end;
					size = 0;
				}

				strncpy( buffer + size, (word + "\n").c_str(), length + 1 );
			}

			uint64_t position = end;
			end = end + length + 1;

			// TODO se viene implementata la versione full parallel va rimosso il mutex
			if(exclusive)
				mux.unlock();

			return position;
		}

		/** reads on disk the associated word */
		inline string read( const int64_t position, const bool exclusive = false )
		{
			// TODO se viene implementata la versione full parallel va rimosso il mutex
			if(exclusive)
				mux.lock();

			if(position >= start && position <= end){
				// the string is in the buffer
				char sub_buffer[BUFFER_SIZE];
				uint64_t offset = position - start;

				char* from = buffer + offset;
				char* eos = strchr( from, '\n' );
				p_type size = eos - from;
				memcpy( sub_buffer, from, size );

				// TODO se viene implementata la versione full parallel va rimosso il mutex
				if(exclusive)
					mux.unlock();

				sub_buffer[size] = '\0';

				return string( sub_buffer );
			}
			else{
				// the string is on disk
				ifile.seekg( position, ios_base::beg );
				string word;
				getline( ifile, word );

				// TODO se viene implementata la versione full parallel va rimosso il mutex
				if(exclusive)
					mux.unlock();

				return word;
			}
		}

		/***/
		inline void merge( file_manager* other )
		{
			ifstream* other_s = &(other->ifile);

			string str;

			// count and reserve the number of bytes
			other_s->seekg( 0, ios::end );
			str.reserve( other_s->tellg() );
			other_s->seekg( 0, ios::beg );

			str.assign( (istreambuf_iterator<char>( *other_s )), istreambuf_iterator<char>() );

			ofile.write( str.c_str(), str.size() - 1 );
			ofile.flush();

			other->close();
		}

		/***/
		inline void finish()
		{
			uint64_t size = end - start;
			if(size > 0){
				ofile.write( buffer, size );
				ofile.flush();
			}

			free( buffer );
			start = end = -1;
		}

		/***/
		inline void close()
		{
			ifile.close();
			ofile.close();

			remove( path.c_str() );
		}
};

#endif /* FILE_MANAGER_HPP_ */
