//
// Copyright (c) Johnny Shaw. All rights reserved.
// 
// File:     jxystl/locks.cpp
// Author:   Johnny Shaw
// Abstract: STL wrappers for locks (<mutex>, <shared_mutex>, etc.)
//
#include <fltKernel.h>
#include <jxy/locks.hpp>

jxy::shared_mutex::shared_mutex() noexcept
{
    FltInitializePushLock(&m_PushLock);
}

jxy::shared_mutex::~shared_mutex() noexcept
{
    FltDeletePushLock(&m_PushLock);
}

void jxy::shared_mutex::lock() noexcept
{    
    FltAcquirePushLockExclusive(&m_PushLock);
}

bool jxy::shared_mutex::try_lock() noexcept
{
    lock();
    return true;
}

void jxy::shared_mutex::unlock() noexcept
{    
    FltReleasePushLock(&m_PushLock);
}

void jxy::shared_mutex::lock_shared() noexcept
{    
    FltAcquirePushLockShared(&m_PushLock);
}

bool jxy::shared_mutex::try_lock_shared() noexcept
{
    lock();
    return true;
}

void jxy::shared_mutex::unlock_shared() noexcept
{    
    FltReleasePushLock(&m_PushLock);
}

jxy::shared_mutex::native_handle_type jxy::shared_mutex::native_handle() noexcept
{
    return &m_PushLock;
}
