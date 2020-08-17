#include <iostream>
#include <mutex>
#include <thread>
#include <memory>
#include <list>
#include <condition_variable>
#include <random>
#include <sstream>
//----------------------------
//Defines zone
#define READERS 4
#define WRITERS 8
#define REPEATS 8
#define DELAY_MS 500
//----------------------------
using namespace std;
using namespace chrono_literals;

class concurrent_data{
    int _data;
    std::mutex _mx;
    int reads=0;
    int writes=0;
public:
    concurrent_data(int i=0):_data(i){

    }
    void set(int newData)
    {
        std::lock_guard<mutex> lock(_mx);
        _data = newData;

        cout<<++writes<<":"<<this_thread::get_id()<<" Write: "<<_data<<endl;
    }
    int read()
    {
        std::lock_guard<mutex> lock(_mx);
        cout<<++reads<<":"<<this_thread::get_id()<<" Read: "<<_data<<endl;
        return _data;
    }
};

class reader_task{
    shared_ptr<concurrent_data> _data;

public:
    reader_task(concurrent_data *data):_data(data){
    }
    void operator()(){
        for(int i=0;i<REPEATS;++i){
            _data->read();
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }

    }
};
//writes random value to data
class writer_task{
    shared_ptr<concurrent_data> _data;
    int _number;
    mt19937 generator;
public:
    writer_task(concurrent_data *data):_data(data){
        generator.seed(rand());
    }
    void operator()(){
        for(int i=0;i<REPEATS;++i){
            _number = generator()/25;
            _data->set(_number);
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};
int main()
{
    auto data = new concurrent_data(25);
    list<unique_ptr<thread>> threads;
    for(int i=0;i<READERS;++i){
        threads.push_front(make_unique<thread>(reader_task(data)));
    }
    for(int i=0;i<WRITERS;++i){
        threads.push_front(make_unique<thread>(writer_task(data)));
    }
    //starting threads
    for(auto &thread:threads){
        thread->join();
    }
    return 0;
}
