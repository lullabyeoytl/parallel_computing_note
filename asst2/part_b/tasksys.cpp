#include "tasksys.h"


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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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
    _num_threads = num_threads;
    _stop_flag = false;
    _finished_task_id = -1;
    _finished_task_cv = new std::condition_variable();
    _next_task_id = 0;
    _waiting_queue_mutex = new std::mutex();
    _runnable_queue_mutex = new std::mutex();
    _data_mutex = new std::mutex();
    _threads_pool = new std::thread[_num_threads];  

    for (int i = 0; i < _num_threads; i++) {
        _threads_pool[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::worker_thread_func, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    _stop_flag = true;
    for (int i = 0; i < _num_threads; i++){
        _threads_pool[i].join();
    }

    delete [] _waiting_queue_mutex;
    delete [] _runnable_queue_mutex;
    delete [] _data_mutex;
    delete [] _finished_task_cv;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    std::vector<TaskID> nodeps;
    runAsyncWithDeps(runnable, num_total_tasks, nodeps);
    sync();
}

void TaskSystemParallelThreadPoolSleeping::worker_thread_func() {
    while(!_stop_flag){
        RunnableTask task;
        bool runTask = false;
        _waiting_queue_mutex->lock();
        if(_runnable_tasks.empty()){
            // pop ready task from waiting queue
            _waiting_queue_mutex->lock();
            while(!_waiting_tasks.empty()){
                WaitingTask nxt_task = _waiting_tasks.top();
                if(nxt_task.dep_id > _finished_task_id){
                    break;
                }
                _runnable_tasks.push(RunnableTask(nxt_task.task_id,nxt_task.runnable, nxt_task.num_total_tasks));
                _task_process_map.insert({nxt_task.task_id,std::make_pair(0,nxt_task.num_total_tasks)});
                _waiting_tasks.pop();
            }
        _waiting_queue_mutex->unlock();
        }else{
            // process ready tasks
            task = _runnable_tasks.front();
            if(task.current_task > task.num_total_tasks - 1){
                _runnable_tasks.pop();
            }
            else{
                _runnable_tasks.front().current_task++;
                runTask = true;
            }
        }
        _waiting_queue_mutex->unlock();

        if(runTask){
            task.runnable->runTask(task.current_task, task.num_total_tasks);

            _data_mutex->lock();
            _task_process_map[task.task_id].first++;
            if (_task_process_map[task.task_id].first == _task_process_map[task.task_id].second){
                _task_process_map.erase(task.task_id);
                _finished_task_id = std::max(_finished_task_id, task.task_id);

            }
            _data_mutex->unlock();
        }
    }
}
TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    // TODO: CS149 students will implement this method in Part B.
    TaskID dependce_id = -1;
    if(!deps.empty()){
        dependce_id = *deps.begin();
    }
    WaitingTask task(_next_task_id, dependce_id ,runnable, num_total_tasks);

    _waiting_queue_mutex->lock();
    _waiting_tasks.push(task);
    _waiting_queue_mutex->unlock();

    _next_task_id++;
    return (_next_task_id-1);
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    while (true) {
        std::lock_guard<std::mutex> lock(*_data_mutex);
        if (_finished_task_id + 1 == _next_task_id) break;
   }
    return;
}
