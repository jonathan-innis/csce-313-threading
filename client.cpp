/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"
#include "Histogram.h"
using namespace std;

struct request_struct {
    int n;
    string name;
    SafeBuffer* buffer;
};

struct worker_struct {
    RequestChannel* workerChannel;
    SafeBuffer* buffer;
    Histogram* histogram;
};

void* request_thread_function(void* arg) {
    request_struct* args = (request_struct*) arg;

	for(int i = 0; i < args->n; ++i) {
        args->buffer->push("data " + args->name);
	}
    pthread_exit(NULL);
}

void* worker_thread_function(void* arg) {
    /*
		Fill in this function. 

		Make sure it terminates only when, and not before,
		all the requests have been processed.

		Each thread must have its own dedicated
		RequestChannel. Make sure that if you
		construct a RequestChannel (or any object)
		using "new" that you "delete" it properly,
		and that you send a "quit" request for every
		RequestChannel you construct regardless of
		whether you used "new" for it.
     */
    worker_struct* args = (worker_struct*) arg;

    while(true) {
        string request = args->buffer->pop();
        args->workerChannel->cwrite(request);

        if(request == "quit") {
            args->workerChannel->cwrite("quit");
            delete args->workerChannel;
            break;
        }else{
            string response = args->workerChannel->cread();
            args->histogram->update (request, response);
        }
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
			}
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {

        cout << "n == " << n << endl;
        cout << "w == " << w << endl;

        cout << "CLIENT STARTED:" << endl;  
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl<< flush;

		SafeBuffer request_buffer;
		Histogram hist;

        pthread_t thread1, thread2, thread3;
        request_struct arg1;
        arg1.n = n;
        arg1.name = "John Smith";
        arg1.buffer = &request_buffer;

        request_struct arg2;
        arg2.n = n;
        arg2.name = "Jane Smith";
        arg2.buffer = &request_buffer;

        request_struct arg3;
        arg3.n = n;
        arg3.name = "Joe Smith";
        arg3.buffer = &request_buffer;

        pthread_create(&thread1, NULL, &request_thread_function, (void*) &arg1);
        pthread_create(&thread2, NULL, &request_thread_function, (void*) &arg2);
        pthread_create(&thread3, NULL, &request_thread_function, (void*) &arg3);
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        pthread_join(thread3, NULL);

        cout << "Done populating request buffer" << endl;

        cout << "Pushing quit requests... ";

        pthread_t threads[w];
        worker_struct args[w];
        void* retvals[w];

        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
            chan->cwrite("newchannel");
            string s = chan->cread ();
            RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);
            args[i].workerChannel = workerChannel;
            args[i].buffer = &request_buffer;
            args[i].histogram = &hist;
            pthread_create(&threads[i], NULL, &worker_thread_function, (void*) &args[i]);
        }

        for(int i = 0; i < w; ++i) pthread_join(threads[i], &retvals[i]);
        
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 

		hist.print ();
    }
}
