//
// Created by Admin on 2021/3/22.
//

#include "time_manager.h"
#include <limits>
#include <numeric>
#include <cstdint>
#include <array>
#include "../log/log.h"
#include <cassert>

using namespace wukong;

#define NEW_ARRAY(class_name, num) new class_name[num]
#define DELETE_ARRAY(arr) if(arr != nullptr) delete [] arr; arr = nullptr

const uint32_t kWheelCount = 5;
//const uint32_t kWheelBits[kWheelCount] = { 8, 6, 6, 6, 6 };
const std::array<uint32_t, kWheelCount> kWheelBits = {{10, 8, 8, 6, 0}};

const uint32_t kWheelBitsSum[kWheelCount + 1] = {
        0/*std::accumulate(kWheelBits.begin(), kWheelBits.begin() + 0, 0u)*/,
        std::accumulate(kWheelBits.begin(), kWheelBits.begin() + 1, 0u),
        std::accumulate(kWheelBits.begin(), kWheelBits.begin() + 2, 0u),
        std::accumulate(kWheelBits.begin(), kWheelBits.begin() + 3, 0u),
        std::accumulate(kWheelBits.begin(), kWheelBits.begin() + 4, 0u),
        std::accumulate(kWheelBits.begin(), kWheelBits.end(), 0u)
};
const uint32_t kWheelSize[kWheelCount] = {
        1u << kWheelBits[0],
        1u << kWheelBits[1],
        1u << kWheelBits[2],
        1u << kWheelBits[3],
        1u << kWheelBits[4]
};

#define DOUBLY_LIST_FOR_EACH(pos, head) \
    for(decltype(head) pos = (head)->next; pos != (head); pos = pos->next)

#define DOUBLY_LIST_FOR_EACH_SAFE(pos, head) \
    for(decltype(head) pos = (head)->next, _n_ = pos->next; \
    pos != (head); pos = _n_, _n_ = pos->next)

#define DOUBLY_LIST_POP_FRONT(pos, head) \
    decltype(head) pos = nullptr; \
    if ((head)->next != (head)) { pos = (head)->next; DoublyList::Del(pos); } \

#define SINGLY_LIST_FOR_EACH(pos, head) \
    if(head) for(decltype(head) pos = (head); pos != nullptr; pos = pos->next)

#define SINGLY_LIST_FOR_EACH_SAFE(pos, head) \
    if(head) for(decltype(head) pos = (head), _n_ = pos->next; \
    pos != nullptr; (pos = _n_) ? (_n_ = pos->next) : 0)

#define LIST_HOST_ENTRY(ptr, type, member) \
    ((type*)((char*)(ptr) - ((size_t)&((type*)0)->member)))

struct TimerManager::ListHead
{
    ListHead *next;
    ListHead *prev;

    ListHead()
    {
        next = this;
        prev = this;
    }
};

struct TimerManager::TimerCell
{
    uint32_t count;
    int32_t line_no;
    int64_t interval;

    ListHead link;
    HTIMER timer_uid;
    int64_t next_deadline;

    IArgs *pargs;
    void *user_args;
    const char *file_name;

    Delegate callback;
};

class TimerManager::DoublyList
{
public:
    static inline void InitLinkNode(ListHead *link)
    {
        link->next = link;
        link->prev = link;
    }

    static inline void Add(ListHead *new_node, ListHead *head)
    {
        Add(new_node, head, head->next);
    }

    static inline void AddTail(ListHead *new_node, ListHead *head)
    {
        Add(new_node, head->prev, head);
    }

    static inline void Del(ListHead *entry)
    {
        Del(entry->prev, entry->next);
        entry->prev = nullptr;
        entry->next = nullptr;
    }

    static inline bool Empty(const ListHead *head)
    {
        return head->next == head;
    }

private:
    static inline void Add(ListHead *new_node, ListHead *prev, ListHead *next)
    {
        next->prev = new_node;
        new_node->next = next;
        new_node->prev = prev;
        prev->next = new_node;
    }

    static inline void Del(ListHead *prev, ListHead *next)
    {
        next->prev = prev;
        prev->next = next;
    }
};

class TimerManager::SinglyList
{
public:
    static inline void Add(ListHead *new_node, ListHead *&head)
    {
        new_node->next = head;
        head = new_node;
    }

    static inline void Pop(ListHead *&head)
    {
        head = head->next;
    }

    static inline ListHead *Front(ListHead *head)
    {
        return head;
    }

    static inline bool Empty(const ListHead *head)
    {
        return head == nullptr;
    }
};

class TimerManager::TimerWheel
{
public:
    TimerWheel()
    {
    }

    ~TimerWheel()
    {
        delete[] (list_head_);
        list_head_ = nullptr;
    }

    inline void Init(uint32_t wheel_id)
    {
        assert(wheel_id < kWheelCount);
        wheel_id_ = wheel_id;
        list_head_ = new ListHead[kWheelSize[wheel_id]];
    }

    inline void InsertTimerCell(uint32_t insert_pos, TimerCell *cell)
    {
        DoublyList::AddTail(&cell->link, &list_head_[insert_pos]);
    }

public:
    ListHead *list_head_{nullptr};
    uint32_t wheel_id_{0};
};

TimerManager::TimerManager() :
        now_time_(0),
        timer_pool_singly_list_(nullptr),
        wheels_(nullptr)
{

}

TimerManager::~TimerManager()
{
    UnInit();
}

void TimerManager::Init()
{
    wheels_ = NEW_ARRAY(TimerWheel, kWheelCount);
    for (uint32_t i = 0; i < kWheelCount; ++i)
    {
        wheels_[i].Init(i);
    }
}

void TimerManager::UnInit()
{
    for (auto &it : hash_finder_)
    {
        delete(it);
    }
    hash_finder_.clear();

    DELETE_ARRAY(wheels_);
    wheels_ = nullptr;
}

HTIMER TimerManager::DoSetTimer(const Delegate &callback, IArgs *pargs, void *args, int64_t interval, uint32_t count,
                           const char *file_name, int32_t lineno)
{
    if (interval <= 0)
    {
        callback(pargs, args);
        return INVALID_HTIMER;
    }
    TimerCell *cell = GetFreeTimerCell();

    if (pargs)
    {
        pargs->timer_handler = cell->timer_uid;
    }

    cell->count = count;
    cell->interval = interval;
    cell->pargs = pargs;
    cell->user_args = args;
    cell->callback = callback;
    cell->next_deadline = now_time_ + interval;
    cell->file_name = file_name;
    cell->line_no = lineno;

    InsertTimerCellAtLeastOneFrame(cell);

    return cell->timer_uid;
}

HTIMER TimerManager::DoSetTimer(int64_t delay, const Delegate &callback, IArgs *pargs, void *args,
                                              const char *filename, int32_t lineno)
{
    if (delay <= 0)
    {
        callback(pargs, args);
        delete(pargs);
        return INVALID_HTIMER;
    }
    return DoSetTimer(callback, pargs, args, delay, 1, filename, lineno);
}

int64_t TimerManager::GetTimeLeft(HTIMER timer_uid)
{
    TimerCell *cell = FindTimerCell(timer_uid);
    if (nullptr != cell)
    {
        uint32_t left_count = (cell->count > 1 ? cell->count - 1 : 0);
        int64_t left_time = cell->interval * left_count;
        left_time += (cell->next_deadline > now_time_ ? cell->next_deadline - now_time_ : 0);
        return left_time;
    }
    return 0;
}

float TimerManager::GetTimeLeftFloat(HTIMER timer_uid)
{
    return (static_cast<float>((GetTimeLeft(timer_uid)) / 1000.0f));
}

int64_t TimerManager::GetEndTime(HTIMER timer_uid)
{
    TimerCell *cell = FindTimerCell(timer_uid);
    if (nullptr != cell)
    {
        uint32_t left_count = (cell->count > 1 ? cell->count - 1 : 0);
        int64_t dead_line = (cell->next_deadline > now_time_ ? cell->next_deadline : now_time_);
        dead_line += cell->interval * left_count;
        return dead_line;
    }
    return 0;
}

void TimerManager::KillTimer(HTIMER timer_uid)
{
    if (timer_uid == INVALID_HTIMER)
    {
        return;
    }
    TimerCell *cell = FindTimerCell(timer_uid);
    if (nullptr == cell)
    {
        return;
    }
    cell->count = 0;
    if (cell->link.prev) // Avoid repeatedly recycle
    {
        DoublyList::Del(&cell->link);
        RecycleTimerCell(cell);
    }

}

void TimerManager::Update(int32_t delta_time)
{
    assert(delta_time >= 0);
    cur_time_ = now_time_ + delta_time;
    while (now_time_ < cur_time_)
    {
        auto index = now_time_ & (kWheelSize[0] - 1);
        if (index == 0)
        {
            UpdateWheel(1);
        }
        auto *head = &wheels_[0].list_head_[index];
        while (true)
        {
            DOUBLY_LIST_POP_FRONT(it, head);
            if (!it)
            {
                break;
            }
            TimerCell *cell = LIST_HOST_ENTRY(it, TimerCell, link);
            {
                // FIXME 回调加一个 Timer_handler
                if (!cell->callback(cell->pargs, cell->user_args) || cell->count <= 1)
                {
                    RecycleTimerCell(cell);
                    continue;
                }
                --cell->count;
                cell->next_deadline = now_time_ + cell->interval;
                InsertTimerCellAtLeastOneFrame(cell);
            }
        }
        ++now_time_;
    }
}

HTIMER TimerManager::SetTimer(int64_t delay, ITimerValidCheckArgs *args, void *params, const char *file_name,
                                int32_t lineno)
{
    if (!args)
    {
        return INVALID_HTIMER;
    }

    if (!file_name)
    {
        delete (args);
        return INVALID_HTIMER;
    }

    return DoSetTimer(
            delay,
            DelegateCombination(TimerManager, TimerCallback, this),
            args,
            params,
            file_name,
            lineno);
}

HTIMER
TimerManager::SetTimerWithCounter(ITimerValidCheckArgs *args, void *params, int64_t interval, uint32_t count,
                                    const char *file_name, int32_t lineno)
{
    if (!args)
    {
        return INVALID_HTIMER;
    }

    if (!file_name)
    {
        delete(args);
        return INVALID_HTIMER;
    }

    if (count == 0)
    {
        delete(args);
        LogWarn("SetTimerWithCounter count = [%u], from [%s] , LineNo [%d]", count, file_name, lineno);
        return INVALID_HTIMER;
    }

    return DoSetTimer(
            DelegateCombination(TimerManager, TimerCallback, this),
            args,
            params,
            interval,
            count,
            file_name,
            lineno);
}

bool TimerManager::TimerCallback(IArgs *args, void *params)
{
    if (!args)
    {
        return false;
    }

    auto *check_args = dynamic_cast<ITimerValidCheckArgs*>(args);
    if (!check_args)
    {
        LogWarn("RO_DYNAMIC_CAST ITimerCallbackArgs fail");
        return false;
    }

    if (!check_args->CheckValid())
    {
        LogWarn("ITimerCallbackArgs %u check valid fail",
                   check_args->GetType());
        return false;
    }

    auto &callback = check_args->GetCallback();
    if (!callback)
    {
        LogWarn("ITimerCallbackArgs %u callback is null",
                   check_args->GetType());
        return false;
    }

    return callback(args, params);
}

TimerManager::TimerCell *TimerManager::GetFreeTimerCell()
{
    TimerCell *ret = nullptr;
    if (!SinglyList::Empty(timer_pool_singly_list_))
    {
        ret = LIST_HOST_ENTRY(SinglyList::Front(timer_pool_singly_list_), TimerCell, link);
        SinglyList::Pop(timer_pool_singly_list_);
        ret->timer_uid = (((ret->timer_uid >> 32) + 1) << 32 |
                          (ret->timer_uid & std::numeric_limits<uint32_t>::max()));
    } else
    {
        ret = new TimerCell();
        hash_finder_.push_back(ret);
        ret->timer_uid = 1ull << 32 | hash_finder_.size();
    }
    return ret;
}

TimerManager::TimerCell *TimerManager::FindTimerCell(HTIMER timer_uid) const
{
    size_t hash = (timer_uid & std::numeric_limits<uint32_t>::max()) - 1;
    if (hash >= hash_finder_.size())
    {
        return nullptr;
    }
    TimerCell *ret = hash_finder_[hash];
    if (ret->timer_uid != timer_uid)
    {
        return nullptr;
    }
    return ret;
}

void TimerManager::RecycleTimerCell(TimerCell *cell)
{
    delete (cell->pargs);
    SinglyList::Add(&cell->link, timer_pool_singly_list_);
}

void TimerManager::InsertTimerCellAtLeastOneFrame(TimerCell *cell)
{
    if (!cell)
    {
        return;
    }
    cell->next_deadline = cell->next_deadline > cur_time_ ? cell->next_deadline : cur_time_;
    DoInsertTimerCell(cell);
}

void TimerManager::DoInsertTimerCell(TimerCell *cell)
{
    if (!cell)
    {
        return;
    }
    int64_t dalay = cell->next_deadline - now_time_;
    uint32_t i = 0;
    for (; i < kWheelCount; ++i)
    {
        if (static_cast<uint32_t>(dalay >> kWheelBitsSum[i]) < kWheelSize[i])
        {
            wheels_[i].InsertTimerCell(
                    static_cast<uint32_t>((cell->next_deadline >> kWheelBitsSum[i]) & (kWheelSize[i] - 1)), cell);
            return;
        }
    }
    i = kWheelCount - 1;
    auto index = static_cast<uint32_t>((now_time_ >> kWheelBitsSum[i]) + kWheelSize[i] - 1) & (kWheelSize[i] - 1);
    wheels_[i].InsertTimerCell(index, cell);
}

void TimerManager::UpdateWheel(uint32_t id)
{
    if (id >= kWheelCount)
    {
        return;
    }
    auto index = (now_time_ >> (kWheelBitsSum[id]) & (kWheelSize[id] - 1));
    auto *head = &wheels_[id].list_head_[index];
    DOUBLY_LIST_FOR_EACH_SAFE(it, head)
    {
        TimerCell *cell = LIST_HOST_ENTRY(it, TimerCell, link);
        DoublyList::Del(it);
        DoInsertTimerCell(cell);
    }
    if (index == 0)
    {
        UpdateWheel(id + 1);
    }
}

void TimerManager::DumpTimerCell()
{
    for (auto &cell : hash_finder_)
    {
        printf("source file %s:%d\n", cell->file_name, cell->line_no);
    }
}

