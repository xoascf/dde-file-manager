/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QScopedPointer>
#include "../global/singleton.h"

class DFMSocketInterfacePrivate;
class DFMSocketInterface : public QObject, public Singleton<DFMSocketInterface>
{
    Q_OBJECT
    friend class Singleton<DFMSocketInterface>;
public:
    explicit DFMSocketInterface(QObject *parent = nullptr);
    virtual ~ DFMSocketInterface();

signals:

public slots:
    void showProperty(const QStringList &paths);

private:
    QScopedPointer<DFMSocketInterfacePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMSocketInterface)
};

