/*
 * start.cpp
 *
 *  Created on: 25/set/2015
 *      Author: stefano
 */

#include <iostream>
#include <fstream>
#include <mcheck.h>
#include <vector>
#include <thread>
#include <chrono>
#include <string.h>
#include <signal.h>
#include <algorithm>

#include "Search Tree/searchTree.hpp"
#include "RMQ/rmq.hpp"
#include "Hashing/min_perfect_hashing.hpp"

using namespace std;
using namespace search_tree;

void segfault_sigaction( int signal, siginfo_t *si, void *arg )
{
    (void) arg;
    (void) signal;

    printf( "Caught segfault at address %p\n", si->si_addr );
    exit( 0 );
}

void test_rmq()
{
    vector<uint64_t> input;
    input.push_back( 3 );  //0
    input.push_back( 5 );  //1
    input.push_back( 1 );  //2

    input.push_back( 7 );  //3
    input.push_back( 1 );  //4
    input.push_back( 6 );  //5

    input.push_back( 10 ); //6
    input.push_back( 9 );  //7
    input.push_back( 8 );  //8

    input.push_back( 7 );  //9
    input.push_back( 1 );  //10
    input.push_back( 4 );  //11
    rmq RMQ( input );

    vector<uint64_t> output = { 6, 6, 3, 6, 0 };
    vector<uint64_t> from = { 1, 1, 1, 0, 0 };
    vector<uint64_t> to = { 10, 7, 4, 11, 0 };
    uint64_t res;
    for(uint64_t i = 0; i < output.size(); i++){
            cout << "=============== TEST " << (i + 1) << " INIZIATO ===============" << endl;
            if((res = RMQ.query( from[i], to[i] )) == output[i])
                    cout << "=============== TEST " << (i + 1) << " OK ===============" << endl;
            else{
                    cout << "=============== TEST " << (i + 1) << " FAILED ===============" << endl;
                    cout << "expected: " << output[i] << ", instead of: " << res << endl;
            }
    }

    vector<uint64_t> res_k = RMQ.top_k( 0, 11, 12 );
    for(uint64_t i = 0; i < res_k.size(); i++)
            cout << "VAL[" << i << "] = " << res_k[i] << endl;

    //cout << "RMQ( 1, 10 ) = " << r.query( 1, 10 ) << " = " << input[r.query( 1, 10 )] << endl;
    //cout << "RMQ( 1, 7 ) = " << r.query( 1, 7 ) << " = " << input[r.query( 1, 7 )] << endl;
}

void test_trie()
{
	cout << "INIZIO TEST" << endl;

	//vector<string> dictionary{ "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus", "rumus" };
	vector<string> dictionary{ "romane", "kkomanus", "aomulus", "ddubens", "gguber", "ffubicon", "poubicundus", "jubicundu" };

	sort( dictionary.begin(), dictionary.end() );

	patriciaTrie pt( dictionary, dictionary.size() );
	//patriciaTrie pt;

	/*uint64_t size = dictionary.size();
	for(uint64_t i = 0; i < size; i++){
		pt.addNode( dictionary[i], dictionary[i].size(), 0 );
		cout << "ADDED[" << i << "]: " + dictionary[i] << endl;
		//ptrie.print_all();
	}*/

	pt.finish();

	//return 0;

	cout << "CERCO I PREFISSI..." << endl;

	string pattern = "f";
	vector<string> list;

	pt.strongPrefix( pattern, pattern.length(), list );

	cout << "SIZE: " << list.size() << endl;
	for(unsigned int i = 0; i < list.size(); i++)
		cout << "VALUE[" << i << "]: " + list[i] << endl;

	exit( 0 );
}

void test_ternary_tree( const char* path )
{
	if(path != NULL){
		ifstream file( path );

		vector<string> input;
		input.reserve( 1 << 15 );
		string line;
		while(getline( file, line ))
			input.push_back( line );

		chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
		ternary_search_tree tst( input, input.size() );
		chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
		auto time_span = chrono::duration_cast<chrono::milliseconds>( end - start );

		cout << "TIME: " << (double)(time_span.count()) / 1000. << " seconds" << endl;
	}
	else{
		cout << "START TEST" << endl;

		vector<string> d{ "romane", "kkomanus", "aomulus", "ddubens", "destro", "gguber", "ffubicon", "poubicundus", "jubicundu" };
		//vector<string> d{ "pane", "merenda", "cane", "zebra" };

		ternary_search_tree tst( d, d.size() );

		tst.finish();

		cout << "CERCO I PREFISSI..." << endl;

		const string pattern = "d";
		vector<string> list;

		tst.strongPrefix( pattern, pattern.length(), list );

		cout << "SIZE: " << list.size() << endl;
		for(unsigned int i = 0; i < list.size(); i++)
			cout << "VALUE[" << i << "]: " + list[i] << endl;
	}

	exit( 0 );
}

void test_mph()
{
	min_perfect_hashing<uint32_t> mph;
	uint32_t value1 = 10;
	mph.insert( 'c', &value1 );
	cout << endl;
	uint32_t value2 = 20;
	mph.insert( 'd', &value2 );
	cout << endl;
	uint32_t value3 = 30;
	mph.insert( 'e', &value3 );

	cout << "LOOKUP: " << mph.lookup( 'e' ) << endl;
	uint32_t* res = mph.get( 'e' );
	if(res == nullptr)
		cout << "NULLO" << endl;
	else
		cout << "TROVATO: " << *res << endl;
	cout << "SIZE: " << sizeof( mph ) << endl;

	for(int i = 0; i < 3; i++)
		cout << "I: " << i << ", VALUE: " << *mph.at( i ) << endl;
}

int main( const int argc, const char* argv[] )
{
	//test_trie();
	test_ternary_tree( argv[1] );

	struct sigaction sa;

	memset( &sa, 0, sizeof( sa ) );
	sigemptyset( &sa.sa_mask );
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;
	sigaction( SIGSEGV, &sa, NULL );

	if(argc < 2){
		printf( "Invalid number of arguments.\nUsage: ./start \"path_to_file\"" );
		exit( 0 );
	}

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now(), end = start;
	auto time_span = chrono::duration_cast<chrono::milliseconds>( end - start );

	//patriciaTrie3 ptrie;

	std::vector<uint64_t> scores;

	ifstream file( argv[1] );

	vector<string> input;
	input.reserve( 1 << 15 );
	string line;
	while(getline( file, line ))
		input.push_back( line );

	sort( input.begin(), input.end() );

	//start = chrono::high_resolution_clock::now();

	//patriciaTrie ptrie( input, input.size() );
	patriciaTrie2 ptrie;

	try
	{
		uint64_t size = input.size();

		start = chrono::high_resolution_clock::now();

		for(uint64_t i = 0; i < size; i++){
			ptrie.addNode( input[i], input[i].size(), i );
			cout << "ADDED: " << input[i] << endl;
			//scores.push_back( std::rand() % 1000 );
		}

		//patriciaTrie ptrie2( input );
		//ptrie2.finish();

		/*if(file.is_open()){
			string line;
			uint64_t i = 0;
			while(getline( file, line )){
				ptrie.addNode( line, line.size(), i );
				cout << "ADDED: " << line << endl;
				//scores.push_back( std::rand() % 1000 );
				i++;
			}
		}*/
	}
	catch( exception& e ){
		cout << "ERRORE: " << e.what() << endl;
		return -1;
	}

	ptrie.finish();

	end = chrono::high_resolution_clock::now();
	time_span = chrono::duration_cast<chrono::milliseconds>( end - start );
	cout << "TIME: " << (double)(time_span.count()) / 1000. << " seconds" << endl;

	file.close();

	cout << "START SEARCHING..." << endl;

	this_thread::sleep_for( chrono::seconds( 3 ) );

	string pattern2 = "ca";
	vector<string> list2;

	start = chrono::high_resolution_clock::now();

	ptrie.strongPrefix( pattern2, pattern2.length(), list2 );

	end = chrono::high_resolution_clock::now();
	time_span = chrono::duration_cast<chrono::milliseconds>( end - start );
	cout << "RETRIEVE TIME: " << (double) (time_span.count()) / 1000. << " seconds" << endl;

	cout << "SIZE: " << list2.size() << endl;
	this_thread::sleep_for( chrono::seconds( 5 ) );

	for(unsigned int i = 0; i < list2.size(); i++)
		cout << "VALUE[" << i << "]: " + list2[i] << endl;

	return 0;

	rmq RMQ( scores );

	this_thread::sleep_for( chrono::seconds( 3 ) );
	start = chrono::high_resolution_clock::now();

	// compute top-k
	vector<uint64_t> top = RMQ.top_k( 0, scores.size() - 1, 5000 );

	end = chrono::high_resolution_clock::now();
	time_span = chrono::duration_cast<chrono::milliseconds>( end - start );

	for(uint64_t i = 0; i < top.size(); i++)
		cout << "TOP_K[" << i << "] = " << top[i] << endl;
	cout << "TOP K: " << (double) (time_span.count()) / 1000. << " seconds" << endl;

	return 0;
}
