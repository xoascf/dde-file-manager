/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmeventdispatcher.h"
#include "dfmabstracteventhandler.h"

#include <QList>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QCoreApplication>
#include <QDebug>

DFM_BEGIN_NAMESPACE

class DFMEventDispatcherPrivate
{
public:
    DFMEventDispatcherPrivate(DFMEventDispatcher *qq)
        : q_ptr(qq) {}

    DFMEventDispatcher *q_ptr;
    DFMEventDispatcher::State state;

    void setState(DFMEventDispatcher::State state);

    Q_DECLARE_PUBLIC(DFMEventDispatcher)
};

void DFMEventDispatcherPrivate::setState(DFMEventDispatcher::State state)
{
    Q_Q(DFMEventDispatcher);

    if (this->state == state)
        return;

    this->state = state;
    emit q->stateChanged(state);
}

DFMEventFuture::DFMEventFuture(const QFuture<QVariant> &future)
    : m_future(future)
{

}

DFMEventFuture::DFMEventFuture(const DFMEventFuture &other)
    : m_future(other.m_future)
{

}

void DFMEventFuture::cancel()
{
    m_future.cancel();
}

bool DFMEventFuture::isCanceled() const
{
    return m_future.isCanceled();
}

bool DFMEventFuture::isStarted() const
{
    return m_future.isStarted();
}

bool DFMEventFuture::isFinished() const
{
    return m_future.isFinished();
}

bool DFMEventFuture::isRunning() const
{
    return m_future.isRunning();
}

void DFMEventFuture::waitForFinished()
{
    m_future.waitForFinished();
}

int DFMEventFuture::waitForFinishedWithEventLoop(QEventLoop::ProcessEventsFlags flags) const
{
    auto fun = [flags, this]() {
        QEventLoop loop;
        QFutureWatcher<QVariant> watcher;

        watcher.setFuture(m_future);
        QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);

        return loop.exec(flags);
    };
    // Run in main thread
    return DThreadUtil::runInMainThread(fun);
}

QVariant DFMEventFuture::result() const
{
    return m_future.result();
}

void DFMEventFuture::operator =(const DFMEventFuture &other)
{
    m_future = other.m_future;
}

namespace DFMEventDispatcherData
{
    static QList<DFMAbstractEventHandler*> eventHandler;
    static QList<DFMAbstractEventHandler*> eventFilter;

    Q_GLOBAL_STATIC(QThreadPool, threadPool)
}

class DFMEventDispatcher_ : public DFMEventDispatcher {};
Q_GLOBAL_STATIC(DFMEventDispatcher_, fmedGlobal)

DFMEventDispatcher *DFMEventDispatcher::instance()
{
    return fmedGlobal;
}

DFMEventDispatcher::~DFMEventDispatcher()
{

}

QVariant DFMEventDispatcher::processEvent(const QSharedPointer<DFMEvent> &event)
{
    Q_D(DFMEventDispatcher);

    d->setState(Busy);

    QVariant result;

    for (DFMAbstractEventHandler *handler : DFMEventDispatcherData::eventFilter) {
        if (handler->fmEventFilter(event, &result))
            return result;
    }

    for (DFMAbstractEventHandler *handler : DFMEventDispatcherData::eventHandler) {
        if (handler->fmEvent(event, &result))
            return result;
    }

    d->setState(Normal);

    return result;
}

DFMEventFuture DFMEventDispatcher::processEventAsync(const QSharedPointer<DFMEvent> &event)
{
    QThreadPool *pool = DFMEventDispatcherData::threadPool;

    if (pool->maxThreadCount() <= pool->activeThreadCount()) {
        pool->setMaxThreadCount(pool->maxThreadCount() + 2);
    }

    return DFMEventFuture(QtConcurrent::run(pool, this, static_cast<QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent>&)>(&DFMEventDispatcher::processEvent), event));
}

QVariant DFMEventDispatcher::processEventWithEventLoop(const QSharedPointer<DFMEvent> &event)
{
    const DFMEventFuture &future = processEventAsync(event);

    future.waitForFinishedWithEventLoop();

    return future.result();
}

void DFMEventDispatcher::installEventFilter(DFMAbstractEventHandler *handler)
{
    if (!DFMEventDispatcherData::eventFilter.contains(handler)) {
        DFMEventDispatcherData::eventFilter.append(handler);
    }
}

void DFMEventDispatcher::removeEventFilter(DFMAbstractEventHandler *handler)
{
    DFMEventDispatcherData::eventFilter.removeOne(handler);
}

DFMEventDispatcher::State DFMEventDispatcher::state() const
{
    Q_D(const DFMEventDispatcher);

    return d->state;
}

DFMEventDispatcher::DFMEventDispatcher()
    : d_ptr(new DFMEventDispatcherPrivate(this))
{

}

void DFMEventDispatcher::installEventHandler(DFMAbstractEventHandler *handler)
{
    if (!DFMEventDispatcherData::eventHandler.contains(handler))
        DFMEventDispatcherData::eventHandler.append(handler);
}

void DFMEventDispatcher::removeEventHandler(DFMAbstractEventHandler *handler)
{
    DFMEventDispatcherData::eventFilter.removeOne(handler);
}

DFM_END_NAMESPACE
