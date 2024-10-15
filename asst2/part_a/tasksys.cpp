#include "tasksys.h"
# include <vector>
# include <thread>
#include <mutex>
#include <condition_variable>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //  
    this->_nums_threads_ = num_threads;
    _threads_pool_ = new std::thread[num_threads];
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    delete[] _threads_pool_;
}

void TaskSystemParallelSpawn::worker(IRunnable* runnable, int num_total_tasks,std::mutex* mutex, int * curr_task) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    
    int turn = -1;
    while (turn < num_total_tasks){
        mutex->lock();
        turn = *curr_task;
        *curr_task += 1;
        mutex->unlock();
        if(turn >= num_total_tasks){
            break;
        }
        runnable -> runTask(turn,num_total_tasks);
    }
    /*
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
    */
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::mutex* mutex = new std::mutex();
    int* curr_task = new int;
    *curr_task = 0;
    for (int i = 0; i < _nums_threads_; i++) {
        _threads_pool_[i] = std::thread(&TaskSystemParallelSpawn::worker, this, runnable, num_total_tasks, mutex, curr_task);
    }
    for (int i = 0; i < _nums_threads_; i++) {
        _threads_pool_[i].join();
    }
    delete curr_task;
    delete mutex;
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

TaskState::TaskState(){
    //initialize the state of the task
    mutex = new std::mutex();
    left_tasks = -1;
    num_completed = -1;
    num_total_tasks = -1;
    cv = new std::condition_variable();
    runnable = nullptr;
    finishedMutex = new std::mutex();
}

TaskState::~TaskState(){
    delete mutex;
    delete cv;
    delete finishedMutex;
}

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _task_states = new TaskState;
    _stop = false;
    _threads_pool_ = new std::thread[num_threads];
    _nums_threads_ = num_threads;
    for (int i = 0; i < num_threads; i++) {
        _threads_pool_[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::worker, this);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    _stop = true;
    for (int i = 0; i < _nums_threads_; i++){
        _threads_pool_[i].join();
    }
    delete[] _threads_pool_;
    delete _task_states;
}

void TaskSystemParallelThreadPoolSpinning::worker() {
    int id;
    int total;
    while (!_stop){
        total = _task_states->num_total_tasks;
        id = total - _task_states->left_tasks.load();  // 确保原子读取
        if(id < total){
            _task_states->left_tasks.fetch_sub(1);  // 原子递减
        }
        if (id < total) {
            _task_states->runnable->runTask(id, total);
                 std::lock_guard<std::mutex> lock(*_task_states->mutex);  // RAII风格锁定
                _task_states->num_completed.fetch_add(1);  // 原子加法
            {
                if (_task_states->num_completed.load() == _task_states->num_total_tasks) {
                    _task_states->finishedMutex->lock();
                    _task_states->finishedMutex->unlock();
                    _task_states->cv->notify_all();  // 在持有锁时通知
                }
            } // 自动解锁
        }
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // 确保对共享数据的操作是线程安全的
    std::unique_lock<std::mutex> lock(*_task_states->finishedMutex);
    _task_states->runnable = runnable;
    _task_states->num_total_tasks = num_total_tasks;
    _task_states->left_tasks.store(num_total_tasks);  // 使用原子操作
    _task_states->num_completed.store(0);  // 使用原子操作

    // 在持有锁的情况下等待条件变量
    _task_states->cv->wait(lock, [this]() { return _task_states->num_completed.load() == _task_states->num_total_tasks; });
    lock.unlock();
}


TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _task_states = new TaskState;
    _stop = false;
    _threads_pool_ = new std::thread[num_threads];
    _nums_threads_ = num_threads;
    _hasTaskMutex = new std::mutex();
    _hasTaskCV = new std::condition_variable();
    for (int i = 0; i < num_threads; i++) {
        _threads_pool_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::worker, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _stop = true;
    for (int i = 0; i < _nums_threads_; i++){
        _hasTaskCV->notify_all();
    }
    for (int i = 0; i < _nums_threads_; i++){
        _threads_pool_[i].join();
    }
    delete _task_states;
    delete[] _threads_pool_;
    delete _hasTaskMutex;
}

void TaskSystemParallelThreadPoolSleeping::worker() {
    int id;
    int total;
    while (!_stop){
        _task_states->mutex->lock();
        total = _task_states->num_total_tasks;
        id = total - _task_states->left_tasks.load();  // 确保原子读取
        if(id < total){
            _task_states->left_tasks.fetch_sub(1);  // 原子递减
        }
        _task_states->mutex->unlock();
        if(id <total ){
            _task_states->mutex->unlock();
            _task_states->finishedMutex->lock();
            _task_states->finishedMutex->unlock();
            _task_states->runnable->runTask(id, total);
            _task_states->cv->notify_all();  // 在持有锁时通知
        }
        else{
            std::unique_lock<std::mutex> lock(*_hasTaskMutex);
            _hasTaskCV->wait(lock, [this]() { return _task_states->left_tasks.load() == 0; });
            lock.unlock();
        }
    }
}
void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::unique_lock<std::mutex> lock(*_task_states->finishedMutex);
    _task_states->mutex->lock();
    _task_states->runnable = runnable;
    _task_states->num_completed.store(0);  // 使用原子操作
    _task_states->num_total_tasks = num_total_tasks;
    _task_states->left_tasks.store(num_total_tasks);  // 使用原子操作
    _task_states->mutex->unlock();
    for (int i=0; i<num_total_tasks; i++){
        _hasTaskCV->notify_all();
    }
    /** 
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
    */
   _task_states->cv->wait(lock, [this]() { return _task_states->num_completed.load() == _task_states->num_total_tasks; });
    lock.unlock();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
