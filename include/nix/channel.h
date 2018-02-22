#ifndef NIX_CHANNEL_H_
#define NIX_CHANNEL_H_
#include <nix/config.h>
#include <nix/contract.h>

#include <condition_variable>
#include <mutex>
#include <memory>
namespace nix {

    /// Model from GoLang unbuffered channel
	/// Key feature here is push/pull is a synchronize point, a pair of consumer and producer will be blocked 
	/// until both arrived the synchronize point, then release them at the same time
	/// channel is default constructable and movable, neither copiable nor assignable.
    template<typename T> class channel {
        channel(const channel&) = delete;
        channel& operator=(const channel&) = delete;
        struct imp_t_ {
            std::mutex mutex;
            std::condition_variable cv_producer, cv_consumer, cv_sync_producer;
			std::atomic<T*> tmp = nullptr;
			std::atomic<bool> entered = false;   // defend spurious wakeup
            bool can_move = false;
        };

		std::unique_ptr<imp_t_> m_imp;
        
    public:
        channel() : m_imp(std::make_unique<imp_t_>()) {}

        channel(channel&& rhs) : m_imp(std::move(rhs.imp)) {}
        ~channel(){};

        channel& operator=(channel&& rhs) {
            if (&rhs != this) {
                m_imp = std::move(rhs.m_imp);
            }
            return *this;
        }
		template<typename U, typename = std::enable_if_t<std::is_same_v<std::remove_reference_t<U>, channel<T>>>>
		void swap(U&& rhs){
			using std::swap;
			swap(m_imp, rhs.m_imp);
		}

		/// push t into channel and be blocked until the value was pulled.
        void push(const T& t) {push_(&t, false);}
		/// push t into channel and be blocked until the value was pulled.
        void push(T&& t) { push_(&t, true);}

		/// retrieve value from channel, and release push thread. 
		/// if channel is empty, wait until some thread pushed a value into channel.
        T pull() {return pull(std::function<void(const T&)>());}

		/// same as above, but call f on the retrieved value. 
		/// \notes if copy,  move assignment or f thrown an exception, the push thread won't be released, and the pull thread can retry getting the value.
		/// if f is valid and thrown an exception, 
        template<typename Func> T pull(Func f) {
            std::unique_lock<std::mutex> lock(m_imp->mutex);

            m_imp->cv_consumer.wait(lock, [&] {return m_imp->tmp != nullptr; });

            NIX_EXPECTS(m_imp->tmp);
			if (m_imp->can_move){
				if (f) f(std::move(*m_imp->tmp)); 

				T ret(std::move(*m_imp->tmp));	// Note: save ret as temporary objects instead of return std::move(*m_imp->tmp), because can't guarantee when would the return value be constructed, before or after release the lock.
				m_imp->tmp = nullptr;
				NIX_EXPECTS(!m_imp->tmp);
				m_imp->cv_sync_producer.notify_one();
				return ret;
			}
			else {
				if (f) f(*const_cast<const T*>(m_imp->tmp.load())); 

				T ret(*const_cast<const T*>(m_imp->tmp.load()));	// here ret is nesseary as above.
				m_imp->tmp = nullptr;
				NIX_EXPECTS(!m_imp->tmp);
				m_imp->cv_sync_producer.notify_one();
				return ret;
			}
        }

    private:
        void push_(const T* t, bool can_move) {            
            std::unique_lock<std::mutex> lock(m_imp->mutex);

            m_imp->cv_producer.wait(lock, [&] {return !m_imp->entered; });
            
            m_imp->entered = true;
            m_imp->tmp = const_cast<T*>(t);
            m_imp->can_move = can_move;

            m_imp->cv_consumer.notify_one();
            NIX_EXPECTS(m_imp->tmp);
            m_imp->cv_sync_producer.wait(lock, [&] {return m_imp->tmp == nullptr; });
            NIX_EXPECTS(!m_imp->tmp);
            m_imp->entered = false;

            m_imp->cv_producer.notify_one();
        }

    };
}

#endif // end NIX_CHANNEL_H_
