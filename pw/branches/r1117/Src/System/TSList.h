
#ifndef __TSLIST_H_
#define __TSLIST_H_

#include "interlocked.h"

/*
Copyright (c) 2020 Erik Rigtorp <erik@rigtorp.se>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory> // std::allocator
#include <new>    // std::hardware_destructive_interference_size
#include <stdexcept>
#include <type_traits> // std::enable_if, std::is_*_constructible

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(nodiscard)
#define RIGTORP_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef RIGTORP_NODISCARD
#define RIGTORP_NODISCARD
#endif

namespace rigtorp {

	template <typename T, typename Allocator = std::allocator<T>> class SPSCQueue {

#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
		template <typename Alloc2, typename = void>
		struct has_allocate_at_least : std::false_type {};

		template <typename Alloc2>
		struct has_allocate_at_least<
			Alloc2, std::void_t<typename Alloc2::value_type,
			decltype(std::declval<Alloc2 &>().allocate_at_least(
				size_t{})) >> : std::true_type {};
#endif

	public:
		explicit SPSCQueue(const size_t Allocated,
			const Allocator &allocator = Allocator())
			: capacity_(Allocated), allocator_(allocator) {
			// The queue needs at least one element
			if (capacity_ < 1) {
				capacity_ = 1;
			}
			capacity_++; // Needs one slack element
						 // Prevent overflowing size_t
			if (capacity_ > SIZE_MAX - 2 * kPadding) {
				capacity_ = SIZE_MAX - 2 * kPadding;
			}

#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
			if constexpr (has_allocate_at_least<Allocator>::value) {
				auto res = allocator_.allocate_at_least(capacity_ + 2 * kPadding);
				slots_ = res.ptr;
				capacity_ = res.count - 2 * kPadding;
			}
			else {
				slots_ = std::allocator_traits<Allocator>::allocate(
					allocator_, capacity_ + 2 * kPadding);
			}
#else
			slots_ = std::allocator_traits<Allocator>::allocate(
				allocator_, capacity_ + 2 * kPadding);
#endif

			static_assert(alignof(SPSCQueue<T>) == kCacheLineSize, "");
			static_assert(sizeof(SPSCQueue<T>) >= 3 * kCacheLineSize, "");
			assert(reinterpret_cast<char *>(&readIdx_) -
				reinterpret_cast<char *>(&writeIdx_) >=
				static_cast<std::ptrdiff_t>(kCacheLineSize));
		}

		~SPSCQueue() {
			while (Pick()) {
				Dequeue();
			}
			std::allocator_traits<Allocator>::deallocate(allocator_, slots_,
				capacity_ + 2 * kPadding);
		}

		// non-copyable and non-movable
		SPSCQueue(const SPSCQueue &) = delete;
		SPSCQueue &operator=(const SPSCQueue &) = delete;

		template <typename... Args>
		void emplace(Args &&...args) noexcept(
			std::is_nothrow_constructible<T, Args &&...>::value) {
			static_assert(std::is_constructible<T, Args &&...>::value,
				"T must be constructible with Args&&...");
			auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
			auto nextWriteIdx = writeIdx + 1;
			if (nextWriteIdx == capacity_) {
				nextWriteIdx = 0;
			}
			while (nextWriteIdx == readIdxCache_) {
				readIdxCache_ = readIdx_.load(std::memory_order_acquire);
			}
			new (&slots_[writeIdx + kPadding]) T(std::forward<Args>(args)...);
			writeIdx_.store(nextWriteIdx, std::memory_order_release);
		}

		template <typename... Args>
		RIGTORP_NODISCARD bool try_emplace(Args &&...args) noexcept(
			std::is_nothrow_constructible<T, Args &&...>::value) {
			static_assert(std::is_constructible<T, Args &&...>::value,
				"T must be constructible with Args&&...");
			auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
			auto nextWriteIdx = writeIdx + 1;
			if (nextWriteIdx == capacity_) {
				nextWriteIdx = 0;
			}
			if (nextWriteIdx == readIdxCache_) {
				readIdxCache_ = readIdx_.load(std::memory_order_acquire);
				if (nextWriteIdx == readIdxCache_) {
					return false;
				}
			}
			new (&slots_[writeIdx + kPadding]) T(std::forward<Args>(args)...);
			writeIdx_.store(nextWriteIdx, std::memory_order_release);
			return true;
		}

		void Enqueue(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
			static_assert(std::is_copy_constructible<T>::value,
				"T must be copy constructible");
			emplace(v);
		}

		template <typename P, typename = typename std::enable_if<
			std::is_constructible<T, P &&>::value>::type>
			void Enqueue(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
			emplace(std::forward<P>(v));
		}

		RIGTORP_NODISCARD bool
			try_push(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
			static_assert(std::is_copy_constructible<T>::value,
				"T must be copy constructible");
			return try_emplace(v);
		}

		template <typename P, typename = typename std::enable_if<
			std::is_constructible<T, P &&>::value>::type>
			RIGTORP_NODISCARD bool
			try_push(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
			return try_emplace(std::forward<P>(v));
		}

		RIGTORP_NODISCARD T *Pick() noexcept {
			auto const readIdx = readIdx_.load(std::memory_order_relaxed);
			if (readIdx == writeIdxCache_) {
				writeIdxCache_ = writeIdx_.load(std::memory_order_acquire);
				if (writeIdxCache_ == readIdx) {
					return nullptr;
				}
			}
			return &slots_[readIdx + kPadding];
		}

		void Dequeue() noexcept {
			static_assert(std::is_nothrow_destructible<T>::value,
				"T must be nothrow destructible");
			auto const readIdx = readIdx_.load(std::memory_order_relaxed);
			assert(writeIdx_.load(std::memory_order_acquire) != readIdx &&
				"Can only call Dequeue() after Pick() has returned a non-nullptr");
			slots_[readIdx + kPadding].~T();
			auto nextReadIdx = readIdx + 1;
			if (nextReadIdx == capacity_) {
				nextReadIdx = 0;
			}
			readIdx_.store(nextReadIdx, std::memory_order_release);
		}

		RIGTORP_NODISCARD size_t Size() const noexcept {
			std::ptrdiff_t diff = writeIdx_.load(std::memory_order_acquire) -
				readIdx_.load(std::memory_order_acquire);
			if (diff < 0) {
				diff += capacity_;
			}
			return static_cast<size_t>(diff);
		}

		RIGTORP_NODISCARD bool empty() const noexcept {
			return writeIdx_.load(std::memory_order_acquire) ==
				readIdx_.load(std::memory_order_acquire);
		}

		RIGTORP_NODISCARD size_t Allocated() const noexcept { return capacity_ - 1; }

	private:
#ifdef __cpp_lib_hardware_interference_size
		static constexpr size_t kCacheLineSize =
			std::hardware_destructive_interference_size;
#else
		static constexpr size_t kCacheLineSize = 64;
#endif

		// Padding to avoid false sharing between slots_ and adjacent allocations
		static constexpr size_t kPadding = (kCacheLineSize - 1) / sizeof(T) + 1;

	private:
		size_t capacity_;
		T *slots_;
#if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
		Allocator allocator_[[no_unique_address]];
#else
		Allocator allocator_;
#endif

		// Align to cache line Size in order to avoid false sharing
		// readIdxCache_ and writeIdxCache_ is used to reduce the amount of cache
		// coherency traffic
		alignas(kCacheLineSize)std::atomic<size_t> writeIdx_ = { 0 };
		alignas(kCacheLineSize)size_t readIdxCache_ = 0;
		alignas(kCacheLineSize)std::atomic<size_t> readIdx_ = { 0 };
		alignas(kCacheLineSize)size_t writeIdxCache_ = 0;
	};
} // namespace rigtorp

//#define DEBUG_TSLIST_HEAVY  1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
template<class T> 
class CTSList
{
	struct SData
	{
		T data;
		SData* pNext;
    SData(): data(), pNext(NULL) {}
	};

	SData *pFirst;
	SData *pLast;
#ifdef DEBUG_TSLIST_HEAVY
  mutable int pushThreadID;
  mutable int popThreadID;
#endif
	CTSList ( const CTSList<T> & ) {}
public:
	CTSList()
	{
		pFirst = new SData();
		pLast = pFirst;
#ifdef DEBUG_TSLIST_HEAVY
    pushThreadID = -1;
    popThreadID = -1;
#endif
  }

	virtual ~CTSList()
	{
#ifdef DEBUG_TSLIST_HEAVY
    popThreadID = -1;
#endif
		while ( !IsEmpty() )
			PopFront();
		delete pLast;
	}

	// Thread 1 only
	virtual T* GetBack()
	{
#ifdef DEBUG_TSLIST_HEAVY
    if ( pushThreadID == -1 )
      pushThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)pushThreadID == ::GetCurrentThreadId(), "TSList misuse: pushed by two threads" );
#endif
		return &pLast->data;
	}

	virtual void PushBack()
	{
#ifdef DEBUG_TSLIST_HEAVY
    if ( pushThreadID == -1 )
      pushThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)pushThreadID == ::GetCurrentThreadId(), "TSList misuse: pushed by two threads" );
#endif
    SData* p = pLast;
    pLast = new SData;
		p->pNext = pLast;
	}

	// Thread 2 only
	virtual bool IsEmpty() const
	{
#ifdef DEBUG_TSLIST_HEAVY
    if ( popThreadID == -1 )
      popThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)popThreadID == ::GetCurrentThreadId(), "TSList misuse: popped by two threads" );
#endif

		return pFirst == pLast;
	}

	virtual T* GetFront(int nI = 0)
	{
#ifdef DEBUG_TSLIST_HEAVY
    if ( popThreadID == -1 )
      popThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)popThreadID == ::GetCurrentThreadId(), "TSList misuse: popped by two threads" );
#endif

    SData *p = pFirst;
    while(nI > 0 && p->pNext != NULL)
    {
      p = p->pNext;
      nI--;
    }
    if(nI == 0 && p->pNext != NULL)
    {
		  return &p->data;
    }
    else
    {
      return NULL;
    }
	}

  virtual T const * GetFront(int nI = 0) const
  {
#ifdef DEBUG_TSLIST_HEAVY
    if ( popThreadID == -1 )
      popThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)popThreadID == ::GetCurrentThreadId(), "TSList misuse: popped by two threads" );
#endif

    SData *p = pFirst;
    while(nI > 0 && p->pNext != NULL)
    {
      p = p->pNext;
      nI--;
    }
    if(nI == 0 && p->pNext != NULL)
    {
      return &p->data;
    }
    else
    {
      return NULL;
    }
  }

	virtual void PopFront(int nI = 0)
	{
#ifdef DEBUG_TSLIST_HEAVY
    if ( popThreadID == -1 )
      popThreadID = ::GetCurrentThreadId();
    NI_ASSERT( (DWORD)popThreadID == ::GetCurrentThreadId(), "TSList misuse: popped by two threads" );
#endif

    SData *pTemp = NULL;
    SData **ppNF = &pFirst;
    pTemp = (*ppNF)->pNext;
    NI_ASSERT(pTemp != NULL, "popping from empty list");
    while(nI > 0 && (*ppNF)->pNext != NULL)
    {
      NI_ASSERT((*ppNF)->pNext != NULL, "popping unexistent item");
      ppNF = &(*ppNF)->pNext;
      pTemp = (*ppNF)->pNext;
      nI --;
    }
    NI_ASSERT((*ppNF)->pNext != NULL, "popping from empty list");
		delete (*ppNF);
		(*ppNF) = pTemp;
	}
};
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
@brief Single-producer, single-consumer thread-safe queue

Uses dynamic memory allocation, but holds unused memory in second queue and re-uses it

@param T Containing data type
@param TIn Type of data to pass to Enqueue function. Useful when T is smart-pointer.
@param TOut Type of data to return from Dequeue function. Useful when T is smart-pointer.
*/
template<class T, class TIn = T, class TOut = T> 
class SPSCQueue
{
public:

  /**
  @brief Default constructor

  @param _recycleListSize Maximum number of list nodes to hold in recycle list
  */
  SPSCQueue( int _recycleListSize = 256 )
    : size(0), recycleSize(0), recycleListSize(_recycleListSize), maxSize(0)
  {
    head = new Node();
    tail = head;
    recycleHead = new Node();
    recycleTail = recycleHead;
  }

  /**
  @brief Destructor

  @param p Pointer (defaults to NULL).
  */
  ~SPSCQueue()
  {
    T dataSink;
    while ( Dequeue( dataSink ) )
      ; 

    Purge( 0 );
    NI_ASSERT( head == tail, "queue must be empty on delete" );
    NI_ASSERT( recycleHead == recycleTail, "queue must be empty on delete" );

    delete head;
    delete recycleHead;
  }

  /**
  @brief Put data into queue

  Can be accessed from producer thread only

  @param data Data 

  @return none
  */
  void Enqueue( const TIn& data )
  {
    Node* n = Allocate( data );
    nival::interlocked_exchange_pointer( head->next, n );
    head = n;
    nival::interlocked_increment( size );
    maxSize = max( size, maxSize );
  }

  /**
  @brief Retrieve data from queue

  Can be accessed from consumer thread only 

  @param data Place to hold retrieved value 

  @note when function returns false, contents of data is not changed

  @return false if queue is empty; true otherwise
  */
  bool Dequeue( TOut& data )
  {
    Node* t = tail;
    Node* n = (Node*)(t->next);
    if ( n == 0 )
      return false;
    data = n->data;
    n->Cleanup();
    Recycle( t );
    tail = n;
    nival::interlocked_decrement( size );
    return true;
  }

  /**
  @brief Retrieve data from queue, but do not dequeue

  Can be accessed from consumer thread only 

  @param data Place to hold retrieved value 

  @note when function returns false, contents of data is not changed

  @return false if queue is empty; true otherwise
  */
  bool Pick( TOut& data )
  {
    Node* t = tail;
    Node* n = (Node*)(t->next);
    if ( n == 0 )
      return false;
    data = n->data;
    return true;
  }

  /**
  @brief Get number of elements in queue

  @return number of elements in queue
  */
  int Size() const { return size; }

  /**
  @brief Get number of allocated elements (in both queue and recycle list)

  @return umber of allocated elements
  */
  int Allocated() const { return size + recycleSize; }

private:
  struct Node
  {
    T data;
    volatile Node* next;

    Node( T _data = T() )
      : next( 0 ), data( _data )
    {}
    void Cleanup()
    {
      data.~T();
      new(&data) T(); 
    }
    void Init( T _data )
    {
      new(&data) T( _data );
      next = 0;
    }
  };

  Node* head;
  Node* tail;
  volatile LONG size;
  volatile LONG maxSize;

  Node* recycleHead; 
  Node* recycleTail; 
  volatile LONG recycleSize;
  int recycleListSize;

  void Recycle( Node* n )
  {
    n->next = 0;
    nival::interlocked_exchange_pointer( recycleHead->next, n );
    recycleHead = n;
    nival::interlocked_increment( recycleSize );
  }

  Node* Allocate( const T& data = T() )
  {
    Node* t = recycleTail;
    Node* n = (Node*)(t->next);
    if ( n == 0 )
      return new Node( data );

    recycleTail = n;

    t->Init( data );

    nival::interlocked_decrement( recycleSize );

    if ( recycleSize > recycleListSize )
      Purge( recycleListSize / 2 );

    return t;
  }

  void Purge( int newSize ) 
  {
    while( recycleSize > newSize )
    {
      Node* t = recycleTail;
      Node* n = (Node*)(t->next);
      if ( n == 0 )
        break;

      delete t;
      recycleTail = n;
      nival::interlocked_decrement( recycleSize );
    }
  }
};
#endif   //__TSLIST_H_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

