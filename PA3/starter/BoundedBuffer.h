#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <assert.h>


using namespace std;

class BoundedBuffer
{
private:
	int cap; // max number of items in the buffer
	queue<vector<char>> q;	/* the queue of items in the buffer. Note
	that each item a sequence of characters that is best represented by a vector<char> for 2 reasons:
	1. An STL std::string cannot keep binary/non-printables
	2. The other alternative is keeping a char* for the sequence and an integer length (i.e., the items can be of variable length).
	While this would work, it is clearly more tedious */

	// add necessary synchronization variables and data structures 
	mutex m; // Push function is now doable with mutex

public:
	BoundedBuffer(int _cap){
		cap = _cap;

	}
	~BoundedBuffer(){

	}

	void push(char* data, int len){
		//1. Wait until there is room in the queue (i.e., queue lengh is less than cap)
		// Must be sleeping until there's room here.
		// TBD

		//2. Convert the incoming byte sequence given by data and len into a vector<char>
		vector<char> d (data, data+len); // Makes a vector of char

		//3. Then push the vector at the end of the queue, but not so quickly. Watch out for race condition.
		m.lock(); // Watches for race condition.
		q.push(d);
		m.unlock(); 

		//4. Wake up pop() threads
		// TBD


	}

	int pop(char* buf, int bufcap){
		//1. Wait until the queue has at least 1 item if there's nothing then suspend the thread until there's something to pop.
		// TDB

		// Don't exceed the capacity of the buffer.
		//2. pop the front item of the queue. The popped item is a vector<char>

		m.lock()
		vector<char> d = q.front();
		q.pop();
		m.unlock();

		//3. Convert the popped vector<char> into a char*, copy that into buf, make sure that vector<char>'s length is <= bufcap
		assert(d.size() <= bufcap); // If this statement is false, then it will crash our program. It shouldn't happen but if it does we exit our program. It's a cautionary measure.
		memcpy(d.data(), d.size()); // Memcopy that onto the buffer

		
		//5. Wake up any potentially sleeping push() functions.
		// TBD

		//4. Return the vector's length to the caller so that he knows many bytes were popped
		return d.size();
		// If you're lucky it doesn't crash, we can use it to some extent. 
	}
};

#endif /* BoundedBuffer_ */
