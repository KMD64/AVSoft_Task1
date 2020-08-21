#include <iostream>//I/O streams
#include <mutex>//mutex
#include <thread>//thread
#include <memory>//shared_ptr
#include <list>//list
#include <random>//mt19937
#include <shared_mutex>
#include <sstream>
//----------------------------
//Зона определений
//----------------------------
//Количество читателей
#define READERS 8
//Количество писателей
#define WRITERS 5
//Количество повторов обращения
#define REPEATS 8
//Задержка между обращениями
#define DELAY_MS 500
//----------------------------
using namespace std;
using stm = shared_timed_mutex;

//mutex для синхронизации вывода в cout
static mutex scr_mx;

static chrono::high_resolution_clock hclock;

//Блокирующий контейнер
class concurrent_data{
    int _data;
    mutable stm _mx;

public:
    concurrent_data(int data=0):_data(data){

    }
    void set(int newData)
    {
        unique_lock<stm> lock(_mx);
        this_thread::sleep_for(chrono::milliseconds(1));
        _data = newData;
    }
    int read() const
    {
        shared_lock<stm> lock(_mx);
        this_thread::sleep_for(chrono::milliseconds(1));
        return _data;
    }
};
//Задача чтения значения
class reader_task{
    shared_ptr<concurrent_data> _data;

public:
    reader_task(shared_ptr<concurrent_data> data):_data(data){

    }
    void operator()(){
        for(int i=0;i<REPEATS;++i){
            int result = _data->read();
            auto end = hclock.now();
            {
            unique_lock<mutex> lock(scr_mx);
            cout<<chrono::duration_cast<chrono::nanoseconds>(end.time_since_epoch()).count()<<" Iteration "<<i<<":Reader #"<<this_thread::get_id()<<" read "<<result<<endl;
            }
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};
//Задача записи случайного значения
class writer_task{
    shared_ptr<concurrent_data> _data;
    int _number;
    //random number generator
    mt19937 generator;

public:
    writer_task(shared_ptr<concurrent_data> data):_data(data){
        generator.seed(rand());
    }
    void operator()(){
        for(int i=0;i<REPEATS;++i){
            _number = generator()%25;
            _data->set(_number);
            auto end = hclock.now();
            {
            unique_lock<mutex> lock(scr_mx);
            cout<<chrono::duration_cast<chrono::nanoseconds>(end.time_since_epoch()).count()<<" Iteration "<<i<<":Writer #"<<this_thread::get_id()<<" wrote "<<_number<<endl;
            }
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};
int main()
{
    //Общий контейнер
    auto data = make_shared<concurrent_data>(22);
    //Заполняем список потоков
    list<unique_ptr<thread>> threads;
    for(int i=0;i<READERS;++i){
        threads.push_front(make_unique<thread>(reader_task(data)));
    }
    for(int i=0;i<WRITERS;++i){
        threads.push_front(make_unique<thread>(writer_task(data)));
    }
    data.reset();
    //Запускаем потоки
    for(auto &thread:threads){
        thread->join();
    }
    return 0;
}
