#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <assert.h>
#include <vector>
#include <condition_variable>


using namespace std;

class BoundedBuffer
{
private:
	int cap = 0; // max number of items in the buffer
	queue<vector<char>> q;	/* the queue of items in the buffer. Note
	that each item a sequence of characters that is best represented by a vector<char> for 2 reasons:
	1. An STL std::string cannot keep binary/non-printables
	2. The other alternative is keeping a char* for the sequence and an integer length (i.e., the items can be of variable length).
	While this would work, it is clearly more tedious */

	// add necessary synchronization variables and data structures 
	mutex m; // Push function is now doable with mutex
	condition_variable data_availible; // For wait by the pop, signaled by push functions
	condition_variable slot_availible; // Waited on by the push, signaled by the pop functions

public:
	BoundedBuffer(int _cap){
		cap = _cap;

	}
	~BoundedBuffer(){

	}

	void push(char* data, int len){
		//0. Convert the incoming byte sequence given by data and len into a vector<char>
		vector<char> d (data, data+len); // Makes a vector of char

		//1. Wait until there is room in the queue (i.e., queue lengh is less than cap)
		// Must be sleeping until there's room here.
		// TBD
		// vid2 start
		unique_lock<mutex> l(m);
		slot_availible.wait(l,[this]{ return q.size() < cap;}); // To make sure the queue size is proper, we can only push when queue size is less than capacity, good conditions.
		
		// 0 can also be done here.
		//2. Then push the vector at the end of the queue, but not so quickly. Watch out for race condition.
		q.push(d);
		l.unlock(); 
		// If we forget to unlock, it's also alright because by the time the push function reaches the end, the L will go out of scope and will be deallocated/unlocked. 
		//3. Wake up pop() threads
		// TBD
		// vid2 start
		data_availible.notify_one();


	}

	int pop(char* buf, int bufcap){
		//1. Wait until the queue has at least 1 item if there's nothing then suspend the thread until there's something to pop.
		// TDB
		// Vid2 Start

		unique_lock<mutex> l (m); // Wraps mutex around
		data_availible.wait(l, [this]{return q.size() > 0; } );

		// Don't exceed the capacity of the buffer.
		//2. pop the front item of the queue. The popped item is a vector<char>
		// With unique lock, we don't need to lock it anymore. But we need to unlock it asap before running into race condition, therefore its current unlock is the best possible placement.
		vector<char> d = q.front();
		q.pop();
		l.unlock();

		//3. Convert the popped vector<char> into a char*, copy that into buf, make sure that vector<char>'s length is <= bufcap
		assert(d.size() <= bufcap); // If this statement is false, then it will crash our program. It shouldn't happen but if it does we exit our program. It's a cautionary measure.
		memcpy(buf,d.data(), d.size()); // Memcopy that onto the buffer

		
		//5. Wake up any potentially sleeping push() functions.
		// TBD
		// Vid2 Start
		slot_availible.notify_one();

		//4. Return the vector's length to the caller so that he knows many bytes were popped
		return d.size();
		// If you're lucky it doesn't crash, we can use it to some extent. 
	}
};

#endif /* BoundedBuffer_ */
