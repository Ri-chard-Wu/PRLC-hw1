

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

using namespace std;
using namespace std::chrono;
 




class MyThreadClass
{
public:

    struct entryArg{
        MyThreadClass* objPtr;
        int base;
        int threadId;
    };


    MyThreadClass(int threadNum){
        this->threadNum = threadNum;
        threads = new pthread_t[threadNum];
        
        results = new double[threadNum];
    }


    bool StartInternalThread()
    {
            // printf("[StartInternalThread()]\n");
            
            int ret;
            entryArg* argPtr;
            
            for (long i = 0; i < threadNum; i++){
                
                argPtr = new entryArg;
                argPtr->objPtr = this;
                argPtr->base = 10 * i;
                argPtr->threadId = i;

                ret = pthread_create(&threads[i], NULL, work_entry, (void *)argPtr);
                
                if(ret != 0) printf("[StartInternalThread()] create thread failed.\n");
            }

            return ret;
    }



    static void * work_entry(void *argPtr) {

        entryArg *entryPtr = ((entryArg *)argPtr);
        
        int base = entryPtr->base;
        int threadId = entryPtr->threadId;

        entryPtr->objPtr->work(base, threadId); 

        return NULL;
    }



    void work(int base, int threadId){
        
        double a = 1.0, b=1.00000000001;
        for(unsigned int i = 0; i< 100000; i++){
            for(unsigned int j = 0; j< 10000; j++){
                a *= b;
            }
        }
        
        a += base;
        results[threadId] = a;

        printf("[work()]: threadId: %d, a: %f\n", threadId, a); fflush (stdout);
    }



   void WaitForInternalThreadToExit()
   {
        for (int i = 0; i < threadNum; i++){
            pthread_join(threads[i], NULL);
        }
   }


    void print_restuls(){
        
        printf("\n[print_restuls()] print restuls:\n\n");

        for(int i=0; i<threadNum; i++){
            printf("results[%d]: %f\n", i, results[i]);
        }
    }

    pthread_t* threads;
    int threadNum;
    double* results;
};




int main()
{
    MyThreadClass obj(8);

    auto start = high_resolution_clock::now();
    
    obj.StartInternalThread();
    obj.WaitForInternalThreadToExit();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    fprintf(stdout, "total dt: %d us\n", duration.count());

    obj.print_restuls();

    return 0;
}











// #include<pthread.h>
// #include <stdio.h>
// #include <stdlib.h>


// #define BUF_SIZE 3



// int buffer[BUF_SIZE];							/* shared buffer */
// int wPtr = 0;										/* place to add next element */
// int rPtr = 0;										/* place to remove next element */
// int num = 0;										/* number elements in buffer */
// pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;	/* mutex lock for buffer */
// pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* consumer waits on this cond var */
// pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* producer waits on this cond var */




// void *producer(void *param)
// {
// 	int i;
// 	for (i=1; i<=20; i++) {


// 		pthread_mutex_lock(&m);


// 		while (num == BUF_SIZE){
//             printf("[producer()]: buffer full, wait...\n"); fflush (stdout);
//             pthread_cond_wait(&c_prod, &m);
//             printf("[producer()]: buffer not full now, can continue...\n"); fflush (stdout);
//         }			
			


// 		buffer[wPtr] = i;
// 		wPtr = (wPtr + 1) % BUF_SIZE;
// 		num++;


// 		pthread_mutex_unlock(&m);


// 		pthread_cond_signal(&c_cons);
// 		printf("[producer()]: inserted %d\n", i); fflush (stdout);
// 	}

// 	printf("[producer()]: quiting\n"); fflush (stdout);
//     pthread_exit(NULL);
// }




// void *consumer(void *param)
// {
// 	int i;
// 	while (1) {

// 		pthread_mutex_lock(&m);


// 		while (num == 0){
//             printf("[consume()]: buffer empty, wait...\n"); fflush (stdout);
//             pthread_cond_wait(&c_cons, &m);
//             printf("[consume()]: buffer not empty now, can continue...\n"); fflush (stdout);
//         }
			

// 		i = buffer[rPtr];
// 		rPtr = (rPtr + 1) % BUF_SIZE;
// 		num--;

// 		pthread_mutex_unlock(&m);

// 		pthread_cond_signal(&c_prod);

// 		printf("[consume()]: value %d\n", i); fflush (stdout);
        
// 	}
// }





// int main (int argc, char *argv[])
// {
// 	pthread_t tid1, tid2;		
// 	int i;


// 	if (pthread_create(&tid1, NULL, producer, NULL) != 0) {
// 		fprintf (stderr, "Unable to create producer thread\n");
// 		exit (1);
// 	}

// 	if (pthread_create(&tid2, NULL, consumer, NULL) != 0) {
// 		fprintf (stderr, "Unable to create consumer thread\n");
// 		exit (1);
// 	}



// 	pthread_join(tid1,NULL);
// 	pthread_join(tid2,NULL);
// 	printf ("Parent quiting\n");

//     return 0;
// }













// #include <algorithm>
// #include <chrono>
// #include <iostream>
// #include<vector>
// using namespace std;
// using namespace std::chrono;
 

// int main()
// {

//     vector<int> values(10000);
 
//     auto f = []() -> int { return rand() % 10000; };
//     generate(values.begin(), values.end(), f);

//     auto start = high_resolution_clock::now();
//     sort(values.begin(), values.end());
//     auto stop = high_resolution_clock::now();
//     auto duration = duration_cast<microseconds>(stop - start);
 
//     fprintf(stdout, "dt: %d us\n", duration.count());

//     // cout << "dt: "<< duration.count() << " us" << endl;
    
 
//     return 0;
// }


















// // CPP program to show the implementation of List
// #include <iostream>
// #include <iterator>
// #include <list>
// using namespace std;
  
// // function for printing the elements in a list
// void showlist(list<int> g)
// {
//     // list<int>::iterator it;
//     // for (it = g.begin(); it != g.end(); ++it)
//     //     cout << ", " << *it;
//     // cout << '\n';
//     while (!g.empty()){
//         cout<<g.back()<<", ";
//         g.pop_back();
//     }
//     cout << '\n';
// }
  

// void f(list<int>* clist){
//     for (int i = 0; i < 10; ++i) {
//         clist->push_front(i * 2);
//     }
// }

// // Driver Code
// int main()
// {
  
//     list<int> gqlist1;
//     f(&gqlist1);
  

//     showlist(gqlist1);

//     // int a = gqlist1.front();
//     // gqlist1.pop_front();
//     // cout << "\ngqlist1.front() : " << a <<endl;

//     // a = gqlist1.front();
//     // gqlist1.pop_front();
//     // cout << "\ngqlist1.front() : " << a<<endl;

//     // a = gqlist1.front();
//     // gqlist1.pop_front();
//     // cout << "\ngqlist1.front() : " << a<<endl;
  
//     // showlist(gqlist1);

//     return 0;
// }



















































// #include <queue>

// #include <bitset>
// #include <iostream>

// using namespace std;
  
// // Print the queue
// void showq(queue<int> gq)
// {
//     queue<int> g = gq;
//     while (!g.empty()) {
//         cout << '\t' << g.front();
//         g.pop();
//     }
//     cout << '\n';
// }
  
// // Driver Code
// int main()
// {
//     queue<int> gquiz;
//     gquiz.push(10);
//     gquiz.push(20);
//     gquiz.push(30);
  
//     cout << "The queue gquiz is : ";
//     showq(gquiz);
  
//     printf("\ngquiz.size() : %d", gquiz.size());
//     // cout << "\ngquiz.size() : " << gquiz.size();
//     cout << "\ngquiz.front() : " << gquiz.front();
//     cout << "\ngquiz.back() : " << gquiz.back();
  
//     cout << "\ngquiz.pop() : ";
//     gquiz.pop();
//     showq(gquiz);
  
//     return 0;
// }










// // #include <unistd.h>
// #include <iostream>
// // #include <cstdlib>
// #include <signal.h>
// using namespace std;

// void signal_callback_handler(int signum) {
//    cout << "\nCaught signal " << signum << endl;
//    exit(signum);
// }

// int main(){
//    // Register signal and signal handler
//    signal(SIGINT, signal_callback_handler);
//    while(true){
//       cout << "Program processing..." << endl;
//     //   sleep(1);
//    }
//    return 0;
// }




// #include <iostream>
// #include <unordered_map>
// using namespace std;
// #include <bitset>

// typedef unsigned short poskey;

// void debug_print(char* msg){
//     fprintf(stderr, msg);
// }


// // Driver code
// int main()
// {
    
//     unordered_map<poskey, int> map;
//     Pos pos;

//     pos.row = 3;
//     pos.col = 4;
//     map[pos2key(pos)] = 340;

//     pos.row = 3;
//     pos.col = 5;
//     map[pos2key(pos)] = 350;




//     pos.row = 3;
//     pos.col = 4;
//     cout<<map[pos2key(pos)]<<endl;

//     pos.row = 3;
//     pos.col = 5;
//     cout<<map[pos2key(pos)]<<endl;
    
// }






// // Driver code
// int main()
// {

//     unordered_map<bitset<8>, bool> umap;
    
//     bitset<8> a = 0b1011; 
//     bitset<8> b = 0b000011;
//     bitset<8> c = 0b11011100;

//     umap[a] = true;
//     umap[b] = false;
//     umap[c] = true;

    
//     // cout<<"umap[c]: "<<umap[a]<<endl;
//     // cout<<"umap[c]: "<<umap[b]<<endl;
//     // cout<<"umap[c]: "<<umap[c]<<endl;

//     bitset<8> key = 0b10101010;
//     // bitset<8> key = 0b000011;
//     // if (umap.find(key) == umap.end())
//     //     cout << key << " not found\n\n";
//     // else
//     //     cout << key << " found\n\n";

//     cout<<"umap[c]: "<<umap[a]<<std::endl;

// //   for (auto x : umap)
// //     cout << x.first << " " << x.second << endl;
// }








// enum Dir{
//     up,
//     down,
//     left,
//     right
// };

// void f(Dir dir){
//     if(dir == up) fprintf(stderr, "up\n");
//     else if(dir == right) printf("right\n");
//     else if(dir == down) printf("down\n");
//     else if(dir == left) printf("left\n");
// }

// int main()
// {   

//     int a=4;
//     int b = (a+3)%53;
//     printf("%d\n", b);
// }

// void f(char* s){
//     // s = new char[3];
//     s[0] = 'e';
//     s[1] = 'r';
//     s[2] = 'l';
//     // s = a;
//     // printf("%c\n", s[0]);
//     // printf("%s\n", s);
// }

// // Driver code
// int main()
// {   
//     char* cptr = new char[3];
//     f(cptr);
//     printf("%c\n", cptr[0]);
//     printf("%s\n", cptr);
    

// }



















// #include <bitset>
// // #include <cstddef>
// // #include <cassert>
// #include <iostream>

// using namespace std;

// int main()
// {
//     enum Action{
//         up,
//         right,
//         left,
//         down
//     };

//     Action a;

//     a = left;
//     cout<<a<<endl;
// }


// int main()
// {
 
//     bitset<38> *ptrb;
    
//     ptrb = new bitset<38>;


//     *ptrb = 0b0;

//     ptrb->set(size_t(37), true);
//     ptrb->set(size_t(36), true);
//     ptrb->set(size_t(34), true);
//     ptrb->set(size_t(32), true);
//     ptrb->reset(size_t(34));

//     cout << "*ptrb:" << *ptrb <<endl ;


//     if ((*ptrb)[34] == 1){
//         cout << "is 1\n";
//     }

// }




// int main()
// {
 
//     std::bitset<38> b3 = 0b0;
    
//     b3.set(size_t(37), true);
//     b3.set(size_t(36), true);

//     b3.set(size_t(34), true);
//     b3.set(size_t(32), true);
//     b3.reset(size_t(34));

//     std::cout << "b3:" << b3 <<endl ;

// }



// int main()
// {
//     typedef std::size_t length_t, position_t; // the hints
 
    
//     // constructors:
//     constexpr std::bitset<4> b1;
//     constexpr std::bitset<4> b2{0xA}; // == 0B1010
//     std::bitset<4> b3{"0011"}; // can also be constexpr since C++23
//     std::bitset<8> b4{"ABBA", length_t(4), /*0:*/'A', /*1:*/'B'}; // == 0B0000'0110
 
//     // bitsets can be printed out to a stream:
//     std::cout << "b1:" << b1 << "; b2:" << b2 << "; b3:" << b3 << "; b4:" << b4 << '\n';
 
//     // bitset supports bitwise operations:
//     b3 |= 0b0100; assert(b3 == 0b0111);
//     b3 &= 0b0011; assert(b3 == 0b0011);
//     b3 ^= std::bitset<4>{0b1100}; assert(b3 == 0b1111);
 
//     // operations on the whole set:
//     b3.reset(); assert(b3 == 0);
//     b3.set(); assert(b3 == 0b1111);

//     assert(b3.all() && b3.any() && !b3.none());
//     b3.flip(); assert(b3 == 0);
 
//     // operations on individual bits:
//     b3.set(position_t(1), true); assert(b3 == 0b0010);
//     b3.set(position_t(1), false); assert(b3 == 0);
//     b3.flip(position_t(2)); assert(b3 == 0b0100);
//     b3.reset(position_t(2)); assert(b3 == 0);
 
//     // subscript operator[] is supported:
//     b3[2] = true; assert(true == b3[2]);
 

// }
