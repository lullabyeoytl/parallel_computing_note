#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include  <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <map>


/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

struct WaitingTask
{
    TaskID task_id;
    TaskID dep_id;
    IRunnable* runnable;
    int num_total_tasks;
    WaitingTask(TaskID task_id, TaskID dep_id, IRunnable* runnable, int num_total_tasks): task_id(task_id), dep_id(dep_id), runnable(runnable), num_total_tasks(num_total_tasks) {} ;
    WaitingTask() {}
    bool operator <(const WaitingTask& other) const {
        return task_id > other.task_id;
    }
};

struct RunnableTask
{
    TaskID task_id;
    IRunnable* runnable;
    int current_task;
    int num_total_tasks;
    RunnableTask(TaskID task_id, IRunnable* runnable, int num_total_tasks) : task_id(task_id), runnable(runnable),  num_total_tasks(num_total_tasks) {}
    RunnableTask() {}
};


/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
    private:
        int _num_threads;

        std::map <TaskID, std::pair<int,int> > _task_process_map;
        std::mutex* _data_mutex;

        TaskID _finished_task_id;
        std::condition_variable* _finished_task_cv;
          // the next call to run or runAsyncWithDeps with get this TaskID back
        TaskID _next_task_id;

        bool _stop_flag;

        std::thread* _threads_pool;
        
        // waiting taskqueue , using priority queue to lessen nums of threads to be created
        std::priority_queue<WaitingTask> _waiting_tasks;
        std::mutex* _waiting_queue_mutex;

        std::queue<RunnableTask> _runnable_tasks;
        std::mutex* _runnable_queue_mutex;

        void worker_thread_func();

};

#endif
