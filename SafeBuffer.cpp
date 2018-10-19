#include "SafeBuffer.h"
#include <string>
#include <queue>
using namespace std;

SafeBuffer::SafeBuffer() {
	pthread_mutex_init(&mut, NULL);
}

SafeBuffer::~SafeBuffer() {
	pthread_mutex_destroy(&mut);
}

int SafeBuffer::size() {
	pthread_mutex_lock(&mut);
	int size = q.size();
	pthread_mutex_unlock(&mut);
	return size;
}

void SafeBuffer::push(string str) {
	pthread_mutex_lock(&mut);
	q.push (str);
	pthread_mutex_unlock(&mut);
}

string SafeBuffer::pop() {
	pthread_mutex_lock(&mut);
	string s = q.front();
	q.pop();
	pthread_mutex_unlock(&mut);
	return s;
}
