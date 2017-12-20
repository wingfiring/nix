#include <xirang2/memory.h>

#include <xirang2/heap.h>
//#include <xirang2/backward/atomic.h>
#include <atomic>

#include <typeinfo>

namespace xirang2
{
	namespace
	{
        std::atomic<heap*> g_global_heap = { 0 };
		void* default_global_heap = 0;
		static_assert(sizeof (default_global_heap) == sizeof(plain_heap), "plain_heap size is wrong");

        class dummy_extern_heap : public ext_heap
        {

            virtual void* malloc(std::size_t /*size*/, std::size_t /*alignment*/, const void* /*hint*/ ) {
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual void free(void* /*p*/, std::size_t /*size*/, std::size_t /*alignment */){
                XR_PRE_CONDITION(false && "not implemented");
            }

            virtual heap* underling(){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual bool equal_to(const heap& rhs) const {
                return typeid(*this) == typeid(rhs);
            }

            virtual heap* hook(){ return 0;}

            virtual heap* hook(heap* /*newhook*/){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

        public:
            virtual offset_range allocate(std::size_t /*size*/, std::size_t /*alignment*/, offset_range /*hint*/) {
                XR_PRE_CONDITION(false && "not implemented");
                return offset_range();
            }

            virtual void deallocate(offset_range /*p*/) {
                XR_PRE_CONDITION(false && "not implemented");
            }

            virtual void* track_pin(offset_range /*h*/) {
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual void* pin(offset_range /*h*/) {
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual int track_pin_count(offset_range /*h*/) const{
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual int view_pin_count(offset_range /*h*/) const{
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual int track_unpin(void* /*h*/){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }
            virtual int unpin(void* /*h*/){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual std::size_t write(offset_range /*h*/, const void* /*src*/, std::size_t /*n*/){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual std::size_t read(offset_range, void* /*dest*/, std::size_t){
                XR_PRE_CONDITION(false && "not implemented");
                return 0;
            }

            virtual void sync(offset_range /*h*/){
                XR_PRE_CONDITION(false && "not implemented");
            }
        };

        dummy_extern_heap default_dummy_extern_heap;
	}

	namespace memory
	{
        XR_API void set_global_heap(heap& h) { g_global_heap.store(&h); }

		XR_API heap& get_global_heap() {
            heap * global_heap = g_global_heap.load();
			if (global_heap == 0){
				init_global_heap_once();
                global_heap = g_global_heap.load();
			}
			XR_PRE_CONDITION(global_heap != 0);
			return *global_heap;
		}

        XR_API ext_heap& get_global_ext_heap() {
            return default_dummy_extern_heap;
		}

		XR_API void init_global_heap_once()
		{
            heap* null_heap = 0;
            if (g_global_heap.compare_exchange_strong(null_heap, (heap*)&default_global_heap))
            {
                // it's safe to constrct default_global_heap multi times.
                new (&default_global_heap) plain_heap(memory::multi_thread);
            }
		}
		XR_API extern std::size_t max_alignment()
		{
			return 16;
		}
	}

}

