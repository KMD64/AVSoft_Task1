#include <iostream>//I/O streams
#include <mutex>//mutex
#include <thread>//thread
#include <memory>//shared_ptr
#include <list>//list
#include <random>//mt19937
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

//Блокирующий контейнер
class concurrent_data{
    int _data;
    std::mutex _mx;
    int writes=0;
    int reads=0;
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
//Задача чтения значения
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
//Задача записи случайного значения
class writer_task{
    shared_ptr<concurrent_data> _data;
    int _number;
    //random number generator
    mt19937 generator;
public:
    writer_task(concurrent_data *data):_data(data){
        generator.seed(rand());
    }
    void operator()(){
        for(int i=0;i<REPEATS;++i){
            _number = generator()%25;
            _data->set(_number);
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};
int main()
{
    //Общий контейнер
    auto data = new concurrent_data(22);
    //Заполняем список потоков
    list<unique_ptr<thread>> threads;
    for(int i=0;i<READERS;++i){
        threads.push_front(make_unique<thread>(reader_task(data)));
    }
    for(int i=0;i<WRITERS;++i){
        threads.push_front(make_unique<thread>(writer_task(data)));
    }
    //Запускаем потоки
    for(auto &thread:threads){
        thread->join();
    }
    return 0;
}
