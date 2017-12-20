#ifndef XIRANG2_CHANNEL_H_
#define XIRANG2_CHANNEL_H_
#include <xirang2/config.h>
#include <xirang2/backward/unique_ptr.h>
#include <condition_variable>
#include <mutex>
#include <deque>
namespace xirang2 {

    //
    template<typename T> class channel {
        channel(const channel&) = delete;
        channel& operator=(const channel&) = delete;
    public:
        channel() : m_imp(make_unique<imp_t_>())
        {}
        channel(channel&& rhs) : m_imp(std::move(rhs.imp)) {}
        ~channel() {};

        channel& operator=(channel&& rhs) {
            if (&rhs != this) {
                m_imp = std::move(rhs.m_imp);
            }
            return *this;
        }

        void push(const T& t) {push_(&t, false);}
        void push(T&& t) { push_(&t, true);}

        T pull() {return pull(std::function<void(const T&)>());}
        template<typename Func>
        T pull(Func f) {
            T ret;
            std::unique_lock<std::mutex> lock(m_imp->mutex);

            m_imp->cv_consumer.wait(lock, [&] {return m_imp->tmp != nullptr; });

            XR_PRE_CONDITION(m_imp->tmp);
            if (m_imp->can_move)
                ret = std::move(*m_imp->tmp);
            else
                ret = *const_cast<const T*>(m_imp->tmp);
            m_imp->tmp = nullptr;
            if (f)
                f(ret);
            XR_PRE_CONDITION(!m_imp->tmp);
            m_imp->cv_sync_producer.notify_one();
            return ret;
        }

    private:
        void push_(const T* t, bool can_move) {            
            std::unique_lock<std::mutex> lock(m_imp->mutex);

            m_imp->cv_producer.wait(lock, [&] {return !m_imp->entered; });
            
            m_imp->entered = true;
            m_imp->tmp = const_cast<T*>(t);
            m_imp->can_move = can_move;

            m_imp->cv_consumer.notify_one();
            XR_PRE_CONDITION(m_imp->tmp);
            m_imp->cv_sync_producer.wait(lock, [&] {return m_imp->tmp == nullptr; });
            XR_PRE_CONDITION(!m_imp->tmp);
            m_imp->entered = false;

            m_imp->cv_producer.notify_one();
        }

        struct imp_t_ {
            std::mutex mutex;
            std::condition_variable cv_producer, cv_consumer, cv_sync_producer;
            T* tmp = nullptr;
            bool entered = false;   // defend spurious wakeup
            bool can_move = false;
        };

        unique_ptr<imp_t_> m_imp;
        
    };

    /*
    template<typename T, typename ...U> 
    struct select {
        select();

        template<typename C, typename Func>
        select& operator()(C& ch, Func f) {

        }
    };


        channel<int> ch_int;
        channel<string> ch_str;

        auto on_int_in = ...;
        auto on_str_in = ...;
        auto on_int_out = ...;
        auto on_str_out = ...;

        auto select = select.on_pull(ch_int, on_int_in).on_push(ch_str, on_str_out);
        select.wait();
    
    
    */
}

#endif // end XIRANG2_CHANNEL_H_
