//
// Copyright (c) Johnny Shaw. All rights reserved.
// 
// File:     jxystl/locks.cpp
// Author:   Johnny Shaw
// Abstract: STL wrappers for locks (<mutex>, <shared_mutex>, etc.)
//
// I chose to have one "locks.hpp" file rather than mutex, shared_mutex, etc. 
// These could be moved to their relevant files to have parity with the standard.
// But I personally find no reason to make that distinction.
//
// jxylib               STL equivalent       Primitive for NT Kernel 
// ---------------------------------------------------------------------------
// jxy::shared_mutex    std::shared_mutex    EX_PUSH_LOCK
// jxy::mutex           std::mutex           KGUARDED_MUTEX
// jxy::shared_lock     std::shared_lock     n/a
// jxy::unique_lock     std::unique_lock     n/a
//
#pragma once
#include <fltKernel.h>
#include <shared_mutex>

namespace jxy
{

#if (NTDDI_VERSION >= NTDDI_WIN8)
class shared_mutex
{
public:

    using native_handle_type = PEX_PUSH_LOCK;

    shared_mutex() noexcept;
    ~shared_mutex() noexcept;
    void lock() noexcept;
    bool try_lock() noexcept;
    void unlock() noexcept;
    void lock_shared() noexcept;
    bool try_lock_shared() noexcept;
    void unlock_shared() noexcept;
    native_handle_type native_handle() noexcept;

private:

    EX_PUSH_LOCK m_PushLock;

};

#endif  //(NTDDI_VERSION >= NTDDI_WIN8)

template <ULONG t_PoolTag>
class mutex
{
public:

    using native_handle_type = PKGUARDED_MUTEX;

    mutex() noexcept(false)
    {
        m_GuardedMutex = static_cast<PKGUARDED_MUTEX>(
            ExAllocatePoolWithTag(NonPagedPoolNx,
                                  sizeof(*m_GuardedMutex),
                                  t_PoolTag));
        if (!m_GuardedMutex)
        {
            throw std::bad_alloc();
        }

        KeInitializeGuardedMutex(m_GuardedMutex);
    }

    ~mutex() noexcept
    {
        if (m_GuardedMutex)
        {
            ExFreePoolWithTag(m_GuardedMutex, t_PoolTag);
        }
    }

    void lock() noexcept
    {
        KeAcquireGuardedMutex(m_GuardedMutex);
    }

    bool try_lock() noexcept
    {
        return (KeTryToAcquireGuardedMutex(m_GuardedMutex) != FALSE ? true : false);
    }

    void unlock() noexcept
    {
        KeReleaseGuardedMutex(m_GuardedMutex);
    }

    native_handle_type native_handle() noexcept
    {
        return m_GuardedMutex;
    }

private:

    PKGUARDED_MUTEX m_GuardedMutex = nullptr;

};

template <ULONG t_PoolTag>
class simple_mutex
{
public:

    using native_handle_type = PRKMUTEX;

    simple_mutex() noexcept(false)
    {
        m_Mutex = static_cast<PRKMUTEX>(
            ExAllocatePoolWithTag(NonPagedPoolNx,
                sizeof(*m_Mutex),
                t_PoolTag));
        if (!m_Mutex)
        {
            throw std::bad_alloc();
        }

        KeInitializeMutex(m_Mutex, NULL);
    }

    ~simple_mutex() noexcept
    {
        if (m_Mutex)
        {
            ExFreePoolWithTag(m_Mutex, t_PoolTag);
        }
    }

    _IRQL_requires_min_(PASSIVE_LEVEL)
    _When_((Timeout == NULL || Timeout->QuadPart != 0), _IRQL_requires_max_(APC_LEVEL))
    _When_((Timeout != NULL && Timeout->QuadPart == 0), _IRQL_requires_max_(DISPATCH_LEVEL))
    NTSTATUS lock(_In_opt_ PLARGE_INTEGER Timeout = nullptr) noexcept 
    {
        auto status = KeWaitForSingleObject(m_Mutex, Executive, KernelMode, FALSE, Timeout);
        return status;
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    bool check() noexcept
    {
        return (KeReadStateMutex(m_Mutex) != FALSE ? true : false);
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    void unlock() noexcept
    {
        KeReleaseMutex(m_Mutex, FALSE);
    }

    native_handle_type native_handle() noexcept
    {
        return m_Mutex;
    }

private:

    PRKMUTEX m_Mutex = nullptr;

};



template <typename T>
using shared_lock = std::shared_lock<T>;

template <typename T>
using unique_lock = std::unique_lock<T>;

}
