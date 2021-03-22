#ifndef WUKONG_TIME_MANAGER_H
#define WUKONG_TIME_MANAGER_H

#include <vector>
#include <list>
#include <deque>
#include <cinttypes>
#include <memory>

namespace wukong
{

#define TIMER_BIND_CALLBACK(T, CALLBACK, INSTANCE) std::bind(&T::CALLBACK, INSTANCE, std::placeholders::_1, std::placeholders::_2)
#define TIMER_BIND_CALLBACK_STC(T, CALLBACK) std::bind(&T::CALLBACK, std::placeholders::_1, std::placeholders::_2)

#define DelegateCombination(T_, Func_, Instance_) (Delegate::registerMethod<T_, &T_::Func_>(Instance_))

#define INVALID_HTIMER 0

    using HTIMER = uint64_t;

    class IArgs
    {
    public:
        virtual ~IArgs()
        {}

        HTIMER timer_handler{INVALID_HTIMER};
    };

    class Delegate
    {
    public:
        Delegate()
                : object_ptr(nullptr), stub_ptr(nullptr)
        {}

        template<class T, bool (T::*TMethod)(IArgs *, void *)>
        static Delegate registerMethod(T *object_ptr)
        {
            Delegate d;
            d.object_ptr = object_ptr;
            d.stub_ptr = &method_stub<T, TMethod>; // #1
            return d;
        }

        bool operator()(IArgs *pargs, void *arg) const
        {
            if (object_ptr == nullptr)
            {
                return false;
            }

            return stub_ptr(object_ptr, pargs, arg);
        }

    private:
        typedef bool(*stub_type)(void *object_ptr, IArgs *, void *arg);

        void *object_ptr;
        stub_type stub_ptr;

        template<class T, bool (T::*TMethod)(IArgs *, void *)>
        static bool method_stub(void *object_ptr, IArgs *pargs, void *arg)
        {
            T *ptr = static_cast<T *>(object_ptr);
            //invocation here as fast as direct method invocation (because its value is known at compile time)
            return (ptr->*TMethod)(pargs, arg);
        }
    };

    class ITimerValidCheckArgs : public IArgs
    {
    public:
        using TimerCallbackFun = std::function<bool(IArgs *, void *)>;

    public:
        ITimerValidCheckArgs(int32_t type, const TimerCallbackFun &callback) :
                type_(type), callback_(callback)
        {}

        virtual ~ITimerValidCheckArgs() = default;

        inline int32_t GetType() const
        { return type_; }

        inline const TimerCallbackFun &GetCallback() const
        { return callback_; }

        //>> 合法性检查
        virtual bool CheckValid()
        { return true; }

    private:
        const int32_t type_;
        const TimerCallbackFun callback_;
    };

    class CommonTimerValidCheckArgs : public ITimerValidCheckArgs
    {
    public:
        CommonTimerValidCheckArgs(const TimerCallbackFun &callback) : ITimerValidCheckArgs(1, callback)
        {}

        virtual ~CommonTimerValidCheckArgs() = default;

        //>> 合法性检查
        virtual bool CheckValid() override
        { return true; }
    };

    class TimerManager
    {
        static const uint32_t kMaxTimerCallbackCount = 1024;
    public:
        TimerManager();

        ~TimerManager();

        void Init();

        void UnInit();

        int64_t GetTimeLeft(HTIMER timer_uid);

        float GetTimeLeftFloat(HTIMER timer_uid);

        int64_t GetEndTime(HTIMER timer_uid);

        //timer_uid:		要Kill的Timer句柄 (SetTimer时返回的值)
        void KillTimer(HTIMER timer_uid);

        void Update(int32_t delta_time);

        //>> delay:		    delay时长(ms)
        //>> args:			合法检查args
        //>> params: 	    回调参数
        //>> file_name:     文件名
        //>> lineno:        行号
        //>> return:		成功返回Timer的句柄，失败返回 INVALID_HTIMER
        HTIMER SetTimer(int64_t delay, ITimerValidCheckArgs *args, void *params, const char *file_name, int32_t lineno);

        //>> args:			合法检查args
        //>> params: 	    回调参数
        //>> interval: 	    定时间隔(ms)
        //>> count:		    触发次数(-1为永远触发)
        //>> file_name:     文件名
        //>> lineno:        行号
        //>> return:		成功返回Timer的句柄，失败返回 INVALID_HTIMER
        //>> 带计数器定时器
        HTIMER SetTimerWithCounter(ITimerValidCheckArgs *args, void *params, int64_t interval, uint32_t count,
                                   const char *file_name, int32_t lineno);

    private:
        //>> 定时器回调
        bool TimerCallback(IArgs *args, void *params);

        HTIMER DoSetTimer(int64_t delay, const Delegate &callback, IArgs *pargs, void *args, const char *filename,
                          int32_t lineno);

        HTIMER DoSetTimer(const Delegate &callback, IArgs *pargs, void *args, int64_t interval, uint32_t count,
                          const char *file_name, int32_t lineno);

    private:
        struct ListHead;
        struct TimerCell;

        class DoublyList;

        class SinglyList;

        class TimerWheel;

        TimerCell *GetFreeTimerCell();

        TimerCell *FindTimerCell(HTIMER timer_uid) const;

        void RecycleTimerCell(TimerCell *cell);

        void InsertTimerCellAtLeastOneFrame(TimerCell *cell);

        void DoInsertTimerCell(TimerCell *cell);

        void UpdateWheel(uint32_t wheel_id);

        void DumpTimerCell();

        int64_t now_time_{INVALID_HTIMER};
        int64_t cur_time_{INVALID_HTIMER};
        std::vector<TimerCell *> hash_finder_;
        ListHead *timer_pool_singly_list_{nullptr};
        TimerWheel *wheels_{nullptr};
    };

}

#endif //WUKONG_TIME_MANAGER_H
